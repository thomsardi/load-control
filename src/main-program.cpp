/**
 * Pin Map
 * 
 * D21 -> DO1
 * D22 -> DO2
 * D23 -> DO3
 * D19 -> SCL
 * D18 -> SDA
 * D36 -> L1_Current_Sense_Ain
 * D39 -> L2_Current_Sense_Ain
 * D34 -> L3_Current_Sense_Ain
 * D32 -> RX2
 * D33 -> TX2
 * D25 -> MISO
 * D26 -> RST
 * D27 -> CS
 * D13 -> SCLK
 * D4 -> MOSI
 */


#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ADS1X15.h>
#include <LoadParameter.h>

#include <flashz-http.hpp>
#include <flashz.hpp>

#include <ModbusServerRTU.h>
#include <loaddefs.h>

#include <CoilData.h>

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"


#define BRIDGE_VOLTAGE_DROP 14
#define TRANSISTOR_VOTAGE_DROP 2
#define CURRENT_GAIN  66  //CC6904SO-20A gain is 63 - 69 mV/A, typical 66 mV/A

const char* TAG = "load-control";

LoadParameter lp;

ModbusServerRTU MBserver(2000);

LoadModbus::modbusRegister buffRegs;
LoadModbus::FeedbackStatus feedbackStatus;
LoadModbus::SystemStatus systemStatus;

loadParamRegister paramRegs;

LoadHandle loadHandle[3];

CoilData myCoils(5);

bool digitalOutput[3] = {0,0,0};

uint16_t testCurrent = 0;

unsigned long lastInc = 0;
unsigned long lastTakenTime = 0;

struct device_pin {
  uint8_t do1 = 21;
  uint8_t do2 = 22;
  uint8_t do3 = 23;
  uint8_t scl = 19;
  uint8_t sda = 18;
  uint8_t currentIn1 = 36;
  uint8_t currentIn2 = 39;
  uint8_t currentIn3 = 34;
  uint8_t rx2 = 32;
  uint8_t tx2 = 33;
  uint8_t miso = 25;
  uint8_t rst = 26;
  uint8_t ss = 27;
  uint8_t sck = 13;
  uint8_t mosi = 4;
} device_pin_t;

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



void setup() {
  // put your setup code here, to run once:
  esp_log_level_set(TAG, ESP_LOG_INFO);

  pinMode(device_pin_t.do1, OUTPUT);
  pinMode(device_pin_t.do2, OUTPUT);
  pinMode(device_pin_t.do3, OUTPUT);

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
  float current[3];
  uint16_t convertedCurrent[3];
  for (size_t i = 0; i < 3; i++)
  {
    current[i] = (loadHandle[i].toCurrent(raw[i])) * 100;
    convertedCurrent[i] = abs(current[i]);
  }

  uint16_t loadVolts = 550;

  loadHandle[0].loop(loadVolts, convertedCurrent[0]);
  loadHandle[1].loop(loadVolts, convertedCurrent[1]);
  loadHandle[2].loop(loadVolts, convertedCurrent[2]);

  for (size_t i = 0; i < 3; i++)
  {
    ESP_LOGI(TAG, "load voltage %d = %d\n", i+1, loadVolts);
    ESP_LOGI(TAG, "signed load current %d = %d\n", i+1, (int)current[i]);
    ESP_LOGI(TAG, "unsigned load current %d = %d\n", i+1, convertedCurrent[i]);
    ESP_LOGI(TAG, "overvoltage flag %d = %d\n", i+1, loadHandle[i].isOvervoltage());
    ESP_LOGI(TAG, "undervoltage flag %d = %d\n", i+1, loadHandle[i].isUndervoltage());
    ESP_LOGI(TAG, "overcurrent flag %d = %d\n", i+1, loadHandle[i].isOvercurrent());
    ESP_LOGI(TAG, "short circuit flag %d = %d\n", i+1, loadHandle[i].isShortCircuit());
  }
  
  // ESP_LOGI(TAG, "action : %d\n", loadHandle[0].getAction());
  // ESP_LOGI(TAG, "flag : %d\n", loadHandle[0].getStatus());
  // ESP_LOGI(TAG, "overvoltage flag : %d\n", loadHandle[0].isOvervoltage());
  // ESP_LOGI(TAG, "undervoltage flag : %d\n", loadHandle[0].isUndervoltage());
  // ESP_LOGI(TAG, "overcurrent flag : %d\n", loadHandle[0].isOvercurrent());
  
  if (myCoils[3])
  {
    ESP_LOGI(TAG, "manual");
    digitalWrite(device_pin_t.do1, myCoils[0]);
    digitalWrite(device_pin_t.do2, myCoils[1]);
    digitalWrite(device_pin_t.do3, myCoils[2]);
    systemStatus.flag.mode = 1;
  }
  else
  {
    systemStatus.flag.mode = 0;
    digitalWrite(device_pin_t.do1, loadHandle[0].getAction());
    myCoils.set(0, loadHandle[0].getAction());
    digitalWrite(device_pin_t.do2, loadHandle[1].getAction());
    myCoils.set(1, loadHandle[1].getAction());
    digitalWrite(device_pin_t.do3, loadHandle[2].getAction());
    myCoils.set(2, loadHandle[2].getAction());
  }

  feedbackStatus.flag.mcb1 = myCoils[0];
  feedbackStatus.flag.mcb2 = myCoils[1];
  feedbackStatus.flag.mcb3 = myCoils[2];

  buffRegs.assignLoadVoltage1(530);
  buffRegs.assignLoadVoltage2(540);
  buffRegs.assignLoadVoltage3(550);
  buffRegs.assignSystemVoltage(580);
  buffRegs.assignLoadCurrent1(current[0]);
  buffRegs.assignLoadCurrent2(current[1]);
  buffRegs.assignLoadCurrent3(current[2]);
  buffRegs.assignFlag1(loadHandle[0].getStatus());
  buffRegs.assignFlag2(loadHandle[1].getStatus());
  buffRegs.assignFlag3(loadHandle[2].getStatus());
  buffRegs.assignFeedbackStatus(feedbackStatus.value);
  buffRegs.assignSystemStatus(systemStatus.value);

  if (myCoils[4])
  {
    ESP_LOGI(TAG, "restart");
  }

  delay(100);
}