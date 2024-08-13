#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ADS1X15.h>
#include <OneButton.h>
#include <LoadParameter.h>

#include <flashz-http.hpp>
#include <flashz.hpp>

#include <ModbusServerRTU.h>

#include <latchhandle.h>
#include <pulseoutput.h>
#include <loaddefs.h>
#include <SerialOta.h>
#include <Update.h>
#include <flashz.hpp>

#include <CoilData.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"


#define BRIDGE_VOLTAGE_DROP 14
#define TRANSISTOR_VOTAGE_DROP 2
#define CURRENT_GAIN  66  //CC6904SO-20A gain is 63 - 69 mV/A, typical 66 mV/A

const char* TAG = "load-control";

QueueHandle_t receiverQueue = xQueueCreate(4, sizeof(OtaData));
TaskHandle_t recvOtaTaskHandle;

/**
 * Pin Map
 * 
 * D36 -> L1_Current_Sense_Ain
 * D39 -> L2_Current_Sense_Ain
 * D34 -> L3_Current_Sense_Ain
 * D32 -> SDA
 * D33 -> SCL
 * D27 -> MCB Feedback 1
 * D26 -> Relay 1 ON
 * D25 -> Relay 1 OFF
 * D13 -> MCB Feedback 2
 * D18 -> Relay 2 ON
 * D19 -> Relay 2 OFF
 * D21 -> MCB Feedback 3
 * D22 -> Relay 3 ON
 * D23 -> Relay 3 OFF
 * D16 -> RX2
 * D17 -> TX2
 */

struct device_pin {
  uint8_t currentIn1 = 36;
  uint8_t currentIn2 = 39;
  uint8_t currentIn3 = 34;
  uint8_t sda = 32;
  uint8_t scl = 33;

  uint8_t mcbFb1 = 27;
  uint8_t relayOn1 = 26;
  uint8_t relayOff1 = 25;

  uint8_t mcbFb2 = 13;
  uint8_t relayOn2 = 18;
  uint8_t relayOff2 = 19;
  
  uint8_t mcbFb3 = 21;
  uint8_t relayOn3 = 22;
  uint8_t relayOff3 = 23;
  
  uint8_t rx2 = 16;
  uint8_t tx2 = 17;
} device_pin_t;


LoadParameter lp;

ModbusServerRTU MBserver(2000);

LoadModbus::modbusRegister buffRegs;
LoadModbus::FeedbackStatus feedbackStatus;
LoadModbus::SystemStatus systemStatus;

loadParamRegister paramRegs;

LoadHandle loadHandle[3];

LatchHandle latchHandle[3];

/**
 * relay[0] -> Relay 1 ON
 * relay[1] -> Relay 1 OFF
 * relay[2] -> Relay 2 ON
 * relay[3] -> Relay 2 OFF
 * relay[4] -> Relay 3 ON
 * relay[5] -> Relay 3 OFF
 */
PulseOutput relay[6];
uint8_t relayFailedCounter[6];

OneButton mcb[3];
// OneButton buttonOn, buttonOff, buttonMode;

CoilData myCoils(9);

bool mcbConnected[3];

bool buttonOnClicked = false;
bool buttonOffClicked = false;
bool buttonModePressed = false;
bool isRestart = false;

uint16_t testCurrent = 0;

unsigned long lastInc = 0;
unsigned long lastTakenTime = 0;

unsigned long lastRelayFailedCheck[0];

// FC_01: act on 0x01 requests - READ_COIL
ModbusMessage FC_01(ModbusMessage request) {
  ModbusMessage response;
// Request parameters are first coil and number of coils to read
  uint16_t start = 0;
  uint16_t numCoils = 0;
  request.get(2, start, numCoils);

  uint16_t offset = 0x1000;

// Are the parameters valid?
  if (start >= offset && (start + numCoils) - offset <= myCoils.coils()) {
    // Looks like it. Get the requested coils from our storage
    vector<uint8_t> coilset = myCoils.slice(start - offset, numCoils);
    // Set up response according to the specs: serverID, function code, number of bytes to follow, packed coils
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)coilset.size(), coilset);
  } else {
    // Something was wrong with the parameters
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
// Return the response
  return response;
}

// FC05: act on 0x05 requests - WRITE_COIL
ModbusMessage FC_05(ModbusMessage request) {
  ModbusMessage response;
// Request parameters are coil number and 0x0000 (OFF) or 0xFF00 (ON)
  uint16_t start = 0;
  uint16_t state = 0;
  request.get(2, start, state);

  uint16_t offset = 0x1000;

// Is the coil number valid?
  if (start >= offset && (start - offset) <= myCoils.coils()) {
    // Looks like it. Is the ON/OFF parameter correct?
    if (state == 0x0000 || state == 0xFF00) {
      // Yes. We can set the coil
      if (myCoils.set(start - offset, state)) {
        // All fine, coil was set.
        response = ECHO_RESPONSE;
      } else {
        // Setting the coil failed
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
      }
    } else {
      // Wrong data parameter
      response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
    }
  } else {
    // Something was wrong with the coil number
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
// Return the response
  return response;
}

// FC03: worker do serve Modbus function code 0x03 (READ_HOLD_REGISTER)
ModbusMessage FC03(ModbusMessage request) {
  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);

  uint16_t offset = 0x1000;

  // Address and words valid? We assume 10 registers here for demo
  if (address >= offset && words && ((address + words) - offset) <= buffRegs.holdingRegister.size()) {
    // lp.printShadow();
    buffRegs.assignHoldingRegister(paramRegs);
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    // Fill response with requested data
    for (uint16_t i = address - offset; i < (address + words) - offset; ++i) {
      response.add(buffRegs.holdingRegister[i]);
    }
  } else {
    // No, either address or words are outside the limits. Set up error response.
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC04: worker do serve Modbus function code 0x04 (READ_INPUT_REGISTER)
ModbusMessage FC04(ModbusMessage request) {
  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);

  uint16_t offset = 0x1000;

  // Address and words valid? We assume 10 registers here for demo
  if (address >= offset && words && ((address + words) - offset) <= buffRegs.inputRegister.size()) {
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    // Fill response with requested data
    for (uint16_t i = address - offset; i < (address + words) - offset; ++i) {
      response.add(buffRegs.inputRegister[i]);
    }
  } else {
    // No, either address or words are outside the limits. Set up error response.
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC06: worker do serve Modbus function code 0x06 (WRITE_HOLD_REGISTER)
ModbusMessage FC06(ModbusMessage request) {
  uint16_t address;           // requested register address
  uint16_t data;              // value to be written
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, data);

  uint16_t offset = 0x1000;

  // Address valid?
  if (address >= offset) {
    lp.writeSingle(address-offset, data);
    lp.getAllParameter(paramRegs);
    // Looks okay. Set up message with serverID, FC, address and data
    response.add(request.getServerID(), request.getFunctionCode(), address, data);
  } else {
    // No, either address or words are outside the limits. Set up error response.
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC0F: act on 0x0F requests - WRITE_MULT_COILS
ModbusMessage FC_0F(ModbusMessage request) {
  ModbusMessage response;
// Request parameters are first coil to be set, number of coils, number of bytes and packed coil bytes
  uint16_t start = 0;
  uint16_t numCoils = 0;
  uint8_t numBytes = 0;
  uint16_t offset = 2;    // Parameters start after serverID and FC
  offset = request.get(offset, start, numCoils, numBytes);

  uint16_t addrOffset = 0x1000;

  // Check the parameters so far
  if (start >= addrOffset && numCoils && (start + numCoils) - addrOffset <= myCoils.coils()) {
    // Packed coils will fit in our storage
    if (numBytes == ((numCoils - 1) >> 3) + 1) {
      // Byte count seems okay, so get the packed coil bytes now
      vector<uint8_t> coilset;
      request.get(offset, coilset, numBytes);
      // Now set the coils
      if (myCoils.set(start - addrOffset, numCoils, coilset)) {
        // All fine, return shortened echo response, like the standard says
        response.add(request.getServerID(), request.getFunctionCode(), start, numCoils);
      } else {
        // Oops! Setting the coils seems to have failed
        response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_FAILURE);
      }
    } else {
      // numBytes had a wrong value
      response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
    }
  } else {
    // The given set will not fit to our coil storage
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC10: worker do serve Modbus function code 0x10 (WRITE_MULT_REGISTERS)
ModbusMessage FC10(ModbusMessage request) {
  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  uint8_t bytes;
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);
  uint16_t index = request.get(6, bytes);

  bool _isAddressRecognized = true;
  OtaData buff;

  switch (address)
  {
  case 0x2000: //upload firmware
    buff.status = FlagStatus::START_FW;
    break;
  case 0x2001: //upload flash
    buff.status = FlagStatus::START_FLASH;
    break;
  case 0x2002: //upload compressed firmware
    buff.status = FlagStatus::PROCESS;
    break;
  case 0x2003: //upload compressed flash
    buff.status = FlagStatus::END;
    break;
  case 0x2004: //upload compressed flash
    buff.status = FlagStatus::START_FW_COMPRESSED;
    break;
  case 0x2005: //upload compressed flash
    buff.status = FlagStatus::START_FLASH_COMPRESSED;
    break;
  case 0x2006: //upload compressed flash
    buff.status = FlagStatus::PROCESS_COMPRESSED;
    break;
  case 0x2007: //upload compressed flash
    buff.status = FlagStatus::END_COMPRESSED;
    break;
  default:
    _isAddressRecognized = false;
    break;
  }

  // Address and words valid? We assume 10 registers here for demo
  if (_isAddressRecognized && words &&  words <= 64) {
    for (size_t i = 0; i < words; i++)
    {
      uint16_t data = 0;
      index = request.get(index, data);
      buff.data[i] = data & 0xFF;
      buff.currentSize++;
    }
    if (xQueueSend(receiverQueue, &buff, 1) == pdTRUE)
    {
      ESP_LOGI(TAG, "data passed\n");
      response.add(request.getServerID(), request.getFunctionCode(), address, words);
    }
    else
    {
      response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_BUSY);
    }
    // Looks okay. Set up message with serverID, FC and length of data
    return response; 
  }

  uint16_t offset = 0x1000;

  // Address and words valid? We assume 10 registers here for demo
  if (address >= offset && words && ((address + words) - offset) <= buffRegs.holdingRegister.size()) {
    std::vector<uint16_t> dataVec;
    dataVec.reserve(128); 
    for (size_t i = 0; i < words; i++)
    {
      uint16_t data = 0;
      index = request.get(index, data);
      dataVec.push_back(data);
    }
    
    lp.writeMultiple(address-offset, dataVec.size(), dataVec.data());
    lp.getAllParameter(paramRegs);
    
    // Looks okay. Set up message with serverID, FC and length of data
    response.add(request.getServerID(), request.getFunctionCode(), address, words);
  } else {
    // No, either address or words are outside the limits. Set up error response.
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

// FC41: worker do serve Modbus function code 0x41 (START_UPLOAD)
ModbusMessage FC41(ModbusMessage request) {
  uint16_t address;           // requested register address
  uint16_t words;             // requested number of registers
  uint8_t bytesCount;
  ModbusMessage response;     // response message to be sent back

  // get request values
  request.get(2, address);
  request.get(4, words);
  uint16_t index = request.get(6, bytesCount);

  bool _isAddressRecognized = true;
  uint16_t offset = 0x2000;
  OtaData buff;

  switch (address)
  {
  case 0x2000: //upload firmware
    buff.status = FlagStatus::START_FW;
    break;
  case 0x2001: //upload flash
    buff.status = FlagStatus::START_FLASH;
    break;
  case 0x2002: //upload compressed firmware
    buff.status = FlagStatus::START_FW_COMPRESSED;
    break;
  case 0x2003: //upload compressed flash
    buff.status = FlagStatus::START_FLASH_COMPRESSED;
    break;
  default:
    _isAddressRecognized = false;
    break;
  }

  // Address and words valid? We assume 10 registers here for demo
  if (_isAddressRecognized && words &&  words <= 64) {
    
    for (size_t i = 0; i < bytesCount; i++)
    {
      uint8_t data = 0;
      index = request.get(index, data);
      buff.currentSize++;
      buff.data[i] = data;
    }
    if (xQueueSend(receiverQueue, &buff, 1) == pdTRUE)
    {
      ESP_LOGI(TAG, "data passed\n");
      response.add(request.getServerID(), request.getFunctionCode(), address, words);
    }
    else
    {
      response.setError(request.getServerID(), request.getFunctionCode(), SERVER_DEVICE_BUSY);
    }
    // Looks okay. Set up message with serverID, FC and length of data
    
  } else {
    // No, either address or words are outside the limits. Set up error response.
    response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
  }
  return response;
}

void mcbLongPressStart1()
{
  // ESP_LOGI(TAG, "pressed");
  mcbConnected[0] = true;
}

void mcbLongPressStop1()
{
  // ESP_LOGI(TAG, "released");
  mcbConnected[0] = false;
}

void mcbLongPressStart2()
{
  mcbConnected[1] = true;
}

void mcbLongPressStop2()
{
  mcbConnected[1] = false;
}

void mcbLongPressStart3()
{
  mcbConnected[2] = true;
}

void mcbLongPressStop3()
{
  mcbConnected[2] = false;
}

// void buttonOnClick()
// {
//   ESP_LOGI(TAG, "button on clicked\n");
//   buttonOnClicked = true;
// }

// void buttonOffClick()
// {
//   ESP_LOGI(TAG, "button on clicked\n");
//   buttonOffClicked = true;
// }

// void buttonModePress()
// {
//   buttonModePressed = true;
// }

// void buttonModeRelease()
// {
//   buttonModePressed = false;
// }

void recvSerialOta(void * pvParameters)
{
  const char *_TAG = "recv task";
  esp_log_level_set(_TAG, ESP_LOG_INFO);
  size_t totalSize = 0;

  while (1)
  {
    OtaData buff;
    if (xQueueReceive(receiverQueue, &buff, portMAX_DELAY) == pdTRUE)
    {
      size_t bytesWritten = 0;
      ESP_LOGI(_TAG, "received %d bytes\n", buff.currentSize);
      FlashZ &fz = FlashZ::getInstance();
      // print_hex_array(_TAG, buff.data, buff.currentSize);
      switch (buff.status)
      {
      case FlagStatus::START_FW :
        ESP_LOGI(_TAG, "Start firmware upload\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        Update.abort();
        if (Update.begin())
        {
          ESP_LOGI(_TAG, "success init update class");
        }
        else
        {
          Update.abort();
          ESP_LOGI(_TAG, "failed init update class");
        }
        bytesWritten = Update.write(buff.data, buff.currentSize); 
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        break;
      case FlagStatus::START_FW_COMPRESSED :
        ESP_LOGI(_TAG, "Start firmware compressed upload\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        fz.abortz();
        if (fz.beginz())
        {
          ESP_LOGI(_TAG, "success init flashz class");
        }
        else
        {
          fz.abortz();
          ESP_LOGI(_TAG, "failed init flashz class");
        }
        bytesWritten = fz.writez(buff.data, buff.currentSize, false); 
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        break;
      case FlagStatus::START_FLASH :
        ESP_LOGI(_TAG, "Start flash upload\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        Update.abort();
        if (Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS))
        {
          ESP_LOGI(_TAG, "success init update class");
        }
        else
        {
          Update.abort();
          ESP_LOGI(_TAG, "failed init update class");
        }
        bytesWritten = Update.write(buff.data, buff.currentSize); 
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        break;
      case FlagStatus::START_FLASH_COMPRESSED :
        ESP_LOGI(_TAG, "Start flash compressed upload\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        fz.abortz();
        if (fz.beginz(UPDATE_SIZE_UNKNOWN, U_SPIFFS))
        {
          ESP_LOGI(_TAG, "success init flashz class");
        }
        else
        {
          fz.abortz();
          ESP_LOGI(_TAG, "failed init flashz class");
        }
        bytesWritten = fz.writez(buff.data, buff.currentSize, false); 
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        break;
      case FlagStatus::PROCESS :
        ESP_LOGI(_TAG, "writing data\n");  
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        bytesWritten = Update.write(buff.data, buff.currentSize);
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);    
        break;
      case FlagStatus::PROCESS_COMPRESSED :
        ESP_LOGI(_TAG, "writing compressed data\n");  
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        bytesWritten = fz.writez(buff.data, buff.currentSize, false);
        totalSize += bytesWritten;
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);    
        break;
      case FlagStatus::END :
        ESP_LOGI(_TAG, "final data\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        isRestart = true;
        totalSize += buff.currentSize;
        bytesWritten = Update.write(buff.data, buff.currentSize);
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        ESP_LOGI(_TAG, "total %d bytes wrote\n", totalSize);
        if (Update.end(true))
        {
          ESP_LOGI(_TAG, "Success upload ota\n");
          isRestart = true;
        }
        else
        {
          ESP_LOGI(_TAG, "Failed upload ota\n");
        }                
        totalSize = 0;
        break;
      case FlagStatus::END_COMPRESSED :
        ESP_LOGI(_TAG, "final compressed data\n");
        // ESP_LOG_BUFFER_HEXDUMP(_TAG, buff.data, buff.currentSize, ESP_LOG_INFO);
        isRestart = true;
        totalSize += buff.currentSize;
        bytesWritten = fz.writez(buff.data, buff.currentSize, true);
        ESP_LOGI(_TAG, "wrote %d bytes\n", bytesWritten);
        ESP_LOGI(_TAG, "total %d bytes wrote\n", totalSize);
        if (fz.endz(true))
        {
          ESP_LOGI(_TAG, "Success upload compressed ota\n");
          isRestart = true;
        }
        else
        {
          ESP_LOGI(_TAG, "Failed upload compressed ota\n");
        }                
        totalSize = 0;
        break;
      default:
        break;
      }
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  esp_log_level_set(TAG, ESP_LOG_INFO);
  xTaskCreate(recvSerialOta, "OTA recv Task", 8192, NULL, 6, &recvOtaTaskHandle);

  mcb[0].setup(device_pin_t.mcbFb1, INPUT_PULLUP, true);
  mcb[0].setDebounceMs(20);
  mcb[0].setClickMs(50);
  mcb[0].setPressMs(100);
  mcb[0].attachLongPressStart(mcbLongPressStart1);
  mcb[0].attachLongPressStop(mcbLongPressStop1);

  mcb[1].setup(device_pin_t.mcbFb2, INPUT_PULLUP, true);
  mcb[1].setDebounceMs(20);
  mcb[1].setClickMs(50);
  mcb[1].setPressMs(100);
  mcb[1].attachLongPressStart(mcbLongPressStart2);
  mcb[1].attachLongPressStop(mcbLongPressStop2);

  mcb[2].setup(device_pin_t.mcbFb3, INPUT_PULLUP, true);
  mcb[2].setDebounceMs(20);
  mcb[2].setClickMs(50);
  mcb[2].setPressMs(100);
  mcb[2].attachLongPressStart(mcbLongPressStart3);
  mcb[2].attachLongPressStop(mcbLongPressStop3);

  // buttonOn.setup(32, INPUT_PULLUP, true);
  // buttonOn.setDebounceMs(20);
  // buttonOn.setClickMs(50);
  // buttonOn.setPressMs(100);
  // buttonOn.attachClick(buttonOnClick);
  // buttonOff.setup(33, INPUT_PULLUP, true);
  // buttonOff.setDebounceMs(20);
  // buttonOff.setClickMs(50);
  // buttonOff.setPressMs(100);
  // buttonOff.attachClick(buttonOffClick);

  // buttonMode.setup(4, INPUT_PULLUP, true);
  // buttonMode.setDebounceMs(20);
  // buttonMode.setClickMs(50);
  // buttonMode.setPressMs(100);
  // buttonMode.attachLongPressStart(buttonModePress);
  // buttonMode.attachLongPressStop(buttonModeRelease);

  latchHandle[0].setup(device_pin_t.relayOn1, device_pin_t.relayOff1, 100, 100);
  latchHandle[1].setup(device_pin_t.relayOn2, device_pin_t.relayOff2, 100, 100);
  latchHandle[2].setup(device_pin_t.relayOn3, device_pin_t.relayOff3, 100, 100);

  Serial.begin(115200);
  lp.begin("load1");
  // lp.clear();
  // while (1)
  // {
  //   /* code */
  // }
  
  ESP_LOGI(TAG, "baudrate bps = %d\n", lp.getBaudrateBps());
  ESP_LOGI(TAG, "written = %d\n", lp.getAllParameter(paramRegs));

  RTUutils::prepareHardwareSerial(Serial2);
  Serial2.begin(lp.getBaudrateBps());

  // Serial2.begin(lp.getBaudrateBps(), SERIAL_8N1, device_pin_t.rx2, device_pin_t.tx2);
  // Wire.begin(device_pin_t.sda, device_pin_t.scl);
  // SPI.begin(device_pin_t.sck, device_pin_t.miso, device_pin_t.mosi, device_pin_t.ss);

  MBserver.registerWorker(lp.getId(), READ_COIL, &FC_01);
  MBserver.registerWorker(lp.getId(), WRITE_COIL, &FC_05);
  MBserver.registerWorker(lp.getId(), WRITE_MULT_COILS, &FC_0F);
  MBserver.registerWorker(lp.getId(), READ_HOLD_REGISTER, &FC03);
  MBserver.registerWorker(lp.getId(), READ_INPUT_REGISTER, &FC04);
  MBserver.registerWorker(lp.getId(), WRITE_MULT_REGISTERS, &FC10);

  MBserver.begin(Serial2);

  /**
   * load paramater from flash memory and pass it into loadHandle
   */
  LoadParamsSetting s;
  // s.loadOverVoltageDisconnect = lp.getOvervoltageDisconnect1();
  // s.loadOvervoltageReconnect = lp.getOvervoltageReconnect1();
  // s.loadUndervoltageDisconnect = lp.getUndervoltageDisconnect1();
  // s.loadUndervoltageReconnect = lp.getUndervoltageReconnect1();
  // s.loadOvercurrentDisconnect = lp.getOvercurrentDisconnect1();
  // s.loadOcDetectionTime = lp.getOvercurrentDetectionTime1();
  // s.loadOcReconnectTime = lp.getOvercurrentReconnectInterval1();
  // s.loadShortCircuittDisconnect = lp.getShortCircuitDisconnect1();
  // s.loadShortCircuitDetectionTime = lp.getShortCircuitDetectionTime1();
  // s.loadShortCircuitReconnectTime = lp.getShortCircuitReconnectInterval1();
  // s.activeLow = lp.getOutputMode1();
  loadHandle[0].setParams(s);

  // s.loadOverVoltageDisconnect = lp.getOvervoltageDisconnect2();
  // s.loadOvervoltageReconnect = lp.getOvervoltageReconnect2();
  // s.loadUndervoltageDisconnect = lp.getUndervoltageDisconnect2();
  // s.loadUndervoltageReconnect = lp.getUndervoltageReconnect2();
  // s.loadOvercurrentDisconnect = lp.getOvercurrentDisconnect2();
  // s.loadOcDetectionTime = lp.getOvercurrentDetectionTime2();
  // s.loadOcReconnectTime = lp.getOvercurrentReconnectInterval2();
  // s.loadShortCircuittDisconnect = lp.getShortCircuitDisconnect2();
  // s.loadShortCircuitDetectionTime = lp.getShortCircuitDetectionTime2();
  // s.loadShortCircuitReconnectTime = lp.getShortCircuitReconnectInterval2();
  // s.activeLow = lp.getOutputMode2();
  loadHandle[1].setParams(s);

  // s.loadOverVoltageDisconnect = lp.getOvervoltageDisconnect3();
  // s.loadOvervoltageReconnect = lp.getOvervoltageReconnect3();
  // s.loadUndervoltageDisconnect = lp.getUndervoltageDisconnect3();
  // s.loadUndervoltageReconnect = lp.getUndervoltageReconnect3();
  // s.loadOvercurrentDisconnect = lp.getOvercurrentDisconnect3();
  // s.loadOcDetectionTime = lp.getOvercurrentDetectionTime3();
  // s.loadOcReconnectTime = lp.getOvercurrentReconnectInterval3();
  // s.loadShortCircuittDisconnect = lp.getShortCircuitDisconnect3();
  // s.loadShortCircuitDetectionTime = lp.getShortCircuitDetectionTime3();
  // s.loadShortCircuitReconnectTime = lp.getShortCircuitReconnectInterval3();
  // s.activeLow = lp.getOutputMode3();
  loadHandle[2].setParams(s);

  lastTakenTime = millis();
  lastInc = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  systemStatus.flag.run = 1;
  int raw[3];
  raw[0] = analogRead(36); // current load 1
  raw[1] = analogRead(39); // current load 2
  raw[2] = analogRead(34); // current load 3

  int16_t loadVolts = analogRead(35) / 6;

  float current[3];
  for (size_t i = 0; i < 3; i++)
  {
    current[i] = (loadHandle[i].toCurrent(raw[i])) * 100;
  }

  for (size_t i = 0; i < 6; i++)
  {
    relay[i].tick();
  }

  for (size_t i = 0; i < 3; i++)
  {
    mcb[i].tick();
  }
    
  // buttonOn.tick();
  // buttonOff.tick();
  // buttonMode.tick();
  loadHandle[0].loop(loadVolts, current[0]);
  loadHandle[1].loop(loadVolts, current[1]);
  loadHandle[2].loop(loadVolts, current[2]);
  latchHandle[0].handle(loadHandle[0].getAction(), mcbConnected[0]);
  latchHandle[1].handle(loadHandle[1].getAction(), mcbConnected[1]);
  latchHandle[2].handle(loadHandle[2].getAction(), mcbConnected[2]);

  // for (size_t i = 0; i < 3; i++)
  // {
  //   ESP_LOGI(TAG, "load voltage %d = %d\n", i+1, loadVolts);
  //   ESP_LOGI(TAG, "signed load current %d = %d\n", i+1, (int16_t)current[i]);
  //   ESP_LOGI(TAG, "unsigned load current %d = %d\n", i+1, (uint16_t)current[i]);
  //   ESP_LOGI(TAG, "overvoltage flag %d = %d\n", i+1, loadHandle[i].isOvervoltage());
  //   ESP_LOGI(TAG, "undervoltage flag %d = %d\n", i+1, loadHandle[i].isUndervoltage());
  //   ESP_LOGI(TAG, "overcurrent flag %d = %d\n", i+1, loadHandle[i].isOvercurrent());
  //   ESP_LOGI(TAG, "short circuit flag %d = %d\n", i+1, loadHandle[i].isShortCircuit());
  // }
  
  // ESP_LOGI(TAG, "action : %d\n", loadHandle[0].getAction());
  // ESP_LOGI(TAG, "flag : %d\n", loadHandle[0].getStatus());
  // ESP_LOGI(TAG, "overvoltage flag : %d\n", loadHandle[0].isOvervoltage());
  // ESP_LOGI(TAG, "undervoltage flag : %d\n", loadHandle[0].isUndervoltage());
  // ESP_LOGI(TAG, "overcurrent flag : %d\n", loadHandle[0].isOvercurrent());

  if (myCoils[6])
  {
    // ESP_LOGI(TAG, "manual");
    for (size_t i = 0; i < 3; i++)
    {
      latchHandle[i].setManual();
    }
    
    for (size_t i = 0; i < 6; i++)
    {
      if (myCoils[i])
      {
        relay[i].set();
        myCoils.set(i, false);
      }
    }
    systemStatus.flag.mode = 1;
  }
  else
  {
    ESP_LOGI(TAG, "auto");
    systemStatus.flag.mode = 0;
    for (size_t i = 0; i < 3; i++)
    {
      latchHandle[i].setAuto();
    }
    feedbackStatus.flag.relayOn1Failed = latchHandle[0].isFailedOn();
    feedbackStatus.flag.relayOff1Failed = latchHandle[0].isFailedOff();
    feedbackStatus.flag.relayOn2Failed = latchHandle[1].isFailedOn();
    feedbackStatus.flag.relayOff2Failed = latchHandle[1].isFailedOff();
    feedbackStatus.flag.relayOn3Failed = latchHandle[2].isFailedOn();
    feedbackStatus.flag.relayOff3Failed = latchHandle[2].isFailedOff();
  }

  feedbackStatus.flag.mcb1 = mcbConnected[0];
  feedbackStatus.flag.mcb2 = mcbConnected[1];
  feedbackStatus.flag.mcb3 = mcbConnected[2];

  buffRegs.assignLoadVoltage1(loadVolts);
  buffRegs.assignLoadVoltage2(loadVolts);
  buffRegs.assignLoadVoltage3(loadVolts);
  buffRegs.assignSystemVoltage(loadVolts);
  buffRegs.assignLoadCurrent1(current[0]);
  buffRegs.assignLoadCurrent2(current[1]);
  buffRegs.assignLoadCurrent3(current[2]);
  buffRegs.assignFlag1(loadHandle[0].getStatus());
  buffRegs.assignFlag2(loadHandle[1].getStatus());
  buffRegs.assignFlag3(loadHandle[2].getStatus());
  buffRegs.assignFeedbackStatus(feedbackStatus.value);
  buffRegs.assignSystemStatus(systemStatus.value);

  if (myCoils[8])
  {
    ESP_LOGI(TAG, "factory reset");
    myCoils.set(8, false);
    lp.reset();
  }
  
  if (myCoils[7] || isRestart)
  {
    ESP_LOGI(TAG, "restart");
    myCoils.set(7, false);
    ESP.restart();
  }
  delay(5);
}