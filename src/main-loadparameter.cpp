#include <Arduino.h>
#include <SPI.h>
#include <LoadParameter.h>

#include <flashz-http.hpp>
#include <flashz.hpp>

const char* TAG = "load-control";

LoadParameter lp;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  lp.begin("load1");
  ESP_LOGI(TAG, "baudrate bps = %d\n", lp.getBaudrateBps());
  Serial.begin(lp.getBaudrateBps());
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}