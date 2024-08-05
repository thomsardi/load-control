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

const char* TAG = "load-handle";

LoadParameter lp;

LoadHandle loadHandle[3];

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

void setup() {
  // put your setup code here, to run once:
  esp_log_level_set(TAG, ESP_LOG_INFO);

  pinMode(device_pin_t.do1, OUTPUT);
  pinMode(device_pin_t.do2, OUTPUT);
  pinMode(device_pin_t.do3, OUTPUT);

  lp.begin("load1");  
  Serial.begin(115200);

  // Serial2.begin(lp.getBaudrateBps(), SERIAL_8N1, device_pin_t.rx2, device_pin_t.tx2);
  // Wire.begin(device_pin_t.sda, device_pin_t.scl);
  // SPI.begin(device_pin_t.sck, device_pin_t.miso, device_pin_t.mosi, device_pin_t.ss);

  LoadParamsSetting loadParam1; //set as default parameter

  loadHandle[0].setParams(loadParam1); //pass parameter into loadhandle[0]

  /**
   * Set custom parameter
   */
  LoadParamsSetting loadParam2;
  loadParam2.loadOverVoltageDisconnect = 580;
  loadParam2.loadOvervoltageReconnect = 570;
  loadParam2.loadUndervoltageDisconnect = 510;
  loadParam2.loadUndervoltageReconnect = 520;
  loadParam2.loadOvercurrentDisconnect = 1500;
  loadParam2.loadOcDetectionTime = 1000;
  loadParam2.loadOcReconnectTime = 5000;
  loadParam2.activeLow = true;

  loadHandle[1].setParams(loadParam2);

  LoadParamsSetting loadParam3; //set as default parameter
  loadParam3.loadOverVoltageDisconnect = lp.getOvervoltageDisconnect3();
  loadParam3.loadOvervoltageReconnect = lp.getOvervoltageReconnect3();
  loadParam3.loadUndervoltageDisconnect = lp.getUndervoltageDisconnect3();
  loadParam3.loadUndervoltageReconnect = lp.getUndervoltageReconnect3();
  loadParam3.loadOvercurrentDisconnect = lp.getOvercurrentDisconnect3();
  loadParam3.loadOcDetectionTime = lp.getOvercurrentDetectionTime3();
  loadParam3.loadOcReconnectTime = lp.getOvercurrentReconnectInterval3();
  loadHandle[2].setParams(loadParam3);
}

void loop() {
  // put your main code here, to run repeatedly:

  int raw[3];
  raw[0] = analogRead(36);
  raw[1] = analogRead(39);
  raw[2] = analogRead(34);
  uint16_t loadVolts = (uint16_t)(raw[0]*0.24);
  // uint16_t loadCurrent = (uint16_t)(raw[1] * 1.22);
  // uint16_t loadCurrent = (uint16_t)(loadHandle[0].toCurrent(raw[1])*100);
  float current = loadHandle[0].toCurrent(raw[1]);
  uint16_t loadCurrent = (uint16_t)(abs(current) * 100);

  ESP_LOGI(TAG, "load voltage = %d\n", loadVolts);
  ESP_LOGI(TAG, "load current = %d\n", loadCurrent);

  loadHandle[0].loop(loadVolts, loadCurrent);
  loadHandle[1].loop(loadVolts, loadCurrent);
  loadHandle[2].loop(loadVolts, loadCurrent);
  digitalWrite(device_pin_t.do1, loadHandle[0].getAction());
  digitalWrite(device_pin_t.do2, loadHandle[1].getAction());
  digitalWrite(device_pin_t.do3, loadHandle[2].getAction());
  
  for (size_t i = 0; i < 3; i++)
  {
    /* code */
    ESP_LOGI(TAG, "load %d action : %d\n", i+1, loadHandle[0].getAction());
    ESP_LOGI(TAG, "flag %d  : %d\n", i+1, loadHandle[0].getStatus());
    ESP_LOGI(TAG, "overvoltage %d flag : %d\n", i+1, loadHandle[0].isOvervoltage());
    ESP_LOGI(TAG, "undervoltage %d flag : %d\n", i+1, loadHandle[0].isUndervoltage());
    ESP_LOGI(TAG, "overcurrent %d flag : %d\n", i+1, loadHandle[0].isOvercurrent());
  }
  
  delay(100);
}