#ifndef LOAD_PARAMETER_H
#define LOAD_PARAMETER_H

#include <Arduino.h>
#include <Preferences.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <vector>
#include "LittleFS.h"

typedef std::array<uint16_t, 23> loadParamRegister;

struct LoadParameterData {
    // uint16_t baudrate = 9600;
    // uint16_t id = 254;
    // uint16_t overvoltageDisconnect1 = 600;  // 600 = 600 x 0.1 = 60V
    // uint16_t overvoltageReconnect1 = 580;
    // uint16_t undervoltageDisconnect1 = 500;
    // uint16_t undervoltageReconnect1 = 510;
    // uint16_t overcurrentDisconnect1 = 1000; // 1000 = 1000 x 0.01 = 10A
    // uint16_t overcurrentDetectionTime1 = 4000; // 4000 ms = 4s
    // uint16_t overcurrentReconnectInterval1 = 4000; // 4000 ms = 4s
    // uint16_t overvoltageDisconnect2 = 600;  // 600 = 600 x 0.1 = 60V
    // uint16_t overvoltageReconnect2 = 580;
    // uint16_t undervoltageDisconnect2 = 500;
    // uint16_t undervoltageReconnect2 = 510;
    // uint16_t overcurrentDisconnect2 = 1000; // 1000 = 1000 x 0.01 = 10A
    // uint16_t overcurrentDetectionTime2 = 4000; // 4000 ms = 4s
    // uint16_t overcurrentReconnectInterval2 = 4000; // 4000 ms = 4s
    // uint16_t overvoltageDisconnect3 = 600;  // 600 = 600 x 0.1 = 60V
    // uint16_t overvoltageReconnect3 = 580;
    // uint16_t undervoltageDisconnect3 = 500;
    // uint16_t undervoltageReconnect3 = 510;
    // uint16_t overcurrentDisconnect3 = 1000; // 1000 = 1000 x 0.01 = 10A
    // uint16_t overcurrentDetectionTime3 = 4000; // 4000 ms = 4s
    // uint16_t overcurrentReconnectInterval3 = 4000; // 4000 ms = 4s
};

class LoadParameter
{
private:
    /* data */
    const char* _TAG = "load control parameter";
    loadParamRegister _shadowRegisters = {
        9600, 254,  // baudrate, id
        600, 580, 500, 510, 1000, 4000, 4000,   // Load 1 : overvoltage disconnect, overvoltage reconnect, undervoltage disconnect, undervoltage reconnect, overcurrent disconnect, overcurrent detection time, overcurrent reconnect interval
        600, 580, 500, 510, 1000, 4000, 4000,
        600, 580, 500, 510, 1000, 4000, 4000,
    };
    String _name;
    void checkUpdatedValue(size_t buffSize, uint16_t* inputParam, uint16_t* deviceParam);
    void copy();
    void createDefault();
    void writeShadow();
    void resetWriteFlag();

    void setBaudrate(uint16_t value);
    void setId(uint16_t value);

    void setOvervoltageDisconnect1(uint16_t value);
    void setOvervoltageReconnect1(uint16_t value);
    void setUndervoltageDisconnect1(uint16_t value);
    void setUndervoltageReconnect1(uint16_t value);
    void setOvercurrentDisconnect1(uint16_t value);
    void setOvercurrentDetectionTime1(uint16_t value);
    void setOvercurrentReconnectInterval1(uint16_t value);

    void setOvervoltageDisconnect2(uint16_t value);
    void setOvervoltageReconnect2(uint16_t value);
    void setUndervoltageDisconnect2(uint16_t value);
    void setUndervoltageReconnect2(uint16_t value);
    void setOvercurrentDisconnect2(uint16_t value);
    void setOvercurrentDetectionTime2(uint16_t value);
    void setOvercurrentReconnectInterval2(uint16_t value);

    void setOvervoltageDisconnect3(uint16_t value);
    void setOvervoltageReconnect3(uint16_t value);
    void setUndervoltageDisconnect3(uint16_t value);
    void setUndervoltageReconnect3(uint16_t value);
    void setOvercurrentDisconnect3(uint16_t value);
    void setOvercurrentDetectionTime3(uint16_t value);
    void setOvercurrentReconnectInterval3(uint16_t value);

public:
    LoadParameter(/* args */);
    void printDefault();
    void printUser();
    void printShadow();

    void begin(String name);
    void save();
    void reset();
    void restart();
    void clear();

    void writeSingle(size_t index, uint16_t value);
    size_t writeMultiple(size_t startIndex, size_t buffSize, uint16_t *buff);

    uint16_t getBaudrate();
    int getBaudrateBps();
    uint16_t getId();
    uint16_t getOvervoltageDisconnect1();
    uint16_t getOvervoltageReconnect1();
    uint16_t getUndervoltageDisconnect1();
    uint16_t getUndervoltageReconnect1();
    uint16_t getOvercurrentDisconnect1();
    uint16_t getOvercurrentDetectionTime1();
    uint16_t getOvercurrentReconnectInterval1();

    uint16_t getOvervoltageDisconnect2();
    uint16_t getOvervoltageReconnect2();
    uint16_t getUndervoltageDisconnect2();
    uint16_t getUndervoltageReconnect2();
    uint16_t getOvercurrentDisconnect2();
    uint16_t getOvercurrentDetectionTime2();
    uint16_t getOvercurrentReconnectInterval2();

    uint16_t getOvervoltageDisconnect3();
    uint16_t getOvervoltageReconnect3();
    uint16_t getUndervoltageDisconnect3();
    uint16_t getUndervoltageReconnect3();
    uint16_t getOvercurrentDisconnect3();
    uint16_t getOvercurrentDetectionTime3();
    uint16_t getOvercurrentReconnectInterval3();

    size_t getAllParameter(loadParamRegister &regs);

    ~LoadParameter();
};




#endif