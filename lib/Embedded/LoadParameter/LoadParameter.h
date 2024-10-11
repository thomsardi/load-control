#ifndef LOAD_PARAMETER_H
#define LOAD_PARAMETER_H

#include <Arduino.h>
#include <Preferences.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <vector>
#include "LittleFS.h"

typedef std::array<uint16_t, 35> loadParamRegister;

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
        6, 254,  // baudrate, id
        600, 580, 508, 515, 1500, 500, 4000, 2000, 10, 4000, 0,   // Load 1 : overvoltage disconnect, overvoltage reconnect, undervoltage disconnect, undervoltage reconnect, overcurrent disconnect, overcurrent detection time, overcurrent reconnect interval, short disconnect, short detection time, shoort reconnect, output mode
        600, 580, 508, 515, 1500, 500, 4000, 2000, 10, 4000, 0,
        600, 580, 508, 515, 1500, 500, 4000, 2000, 10, 4000, 0
    };
    String _name;
    void checkUpdatedValue(size_t buffSize, uint16_t* inputParam, uint16_t* deviceParam); //check if there is updated value
    void copy(); //copy from default to user defined parameter
    void createDefault(); //create default parameter
    void writeShadow(); //write parameter into shadow register
    void resetWriteFlag(); //reset write flag

    void setBaudrate(uint16_t value); //save baudrate into flash
    void setId(uint16_t value); //save id into flash

    void setOvervoltageDisconnect1(uint16_t value); //set overvoltage disconnect 1 into flash
    void setOvervoltageReconnect1(uint16_t value); //set overvoltage reconnect 1 into flash
    void setUndervoltageDisconnect1(uint16_t value); //set undervoltage disconnect 1 into flash
    void setUndervoltageReconnect1(uint16_t value); //set undervoltage reconnect 1 into flash
    void setOvercurrentDisconnect1(uint16_t value); //set overcurrent disconnect 1 into flash
    void setOvercurrentDetectionTime1(uint16_t value); //set overcurrent detection time 1 into flash
    void setOvercurrentReconnectInterval1(uint16_t value); //set overcurrent reconnect 1 into flash
    void setShortCircuitDisconnect1(uint16_t value); //set short circuit disconnect 1 into flash
    void setShortCircuitDetectionTime1(uint16_t value); //set short circuit detection time 1 into flash
    void setShortCircuitReconnectInterval1(uint16_t value); //set short circuit reconnect 1 into flash
    void setOutputMode1(uint16_t value); //set output 1 mode into flash

    void setOvervoltageDisconnect2(uint16_t value); 
    void setOvervoltageReconnect2(uint16_t value);
    void setUndervoltageDisconnect2(uint16_t value);
    void setUndervoltageReconnect2(uint16_t value);
    void setOvercurrentDisconnect2(uint16_t value);
    void setOvercurrentDetectionTime2(uint16_t value);
    void setOvercurrentReconnectInterval2(uint16_t value);
    void setShortCircuitDisconnect2(uint16_t value);
    void setShortCircuitDetectionTime2(uint16_t value);
    void setShortCircuitReconnectInterval2(uint16_t value);
    void setOutputMode2(uint16_t value);

    void setOvervoltageDisconnect3(uint16_t value);
    void setOvervoltageReconnect3(uint16_t value);
    void setUndervoltageDisconnect3(uint16_t value);
    void setUndervoltageReconnect3(uint16_t value);
    void setOvercurrentDisconnect3(uint16_t value);
    void setOvercurrentDetectionTime3(uint16_t value);
    void setOvercurrentReconnectInterval3(uint16_t value);
    void setShortCircuitDisconnect3(uint16_t value);
    void setShortCircuitDetectionTime3(uint16_t value);
    void setShortCircuitReconnectInterval3(uint16_t value);
    void setOutputMode3(uint16_t value);

public:
    LoadParameter(/* args */);
    void printDefault(); //print default parameter from flash
    void printUser(); //print user parameter from flash
    void printShadow(); //print shadow register

    void begin(String name); //begin littelfs namespace
    void save(); //perform save from shadow register to flash
    void reset(); //reset
    void restart(); //restart littlefs
    void clear(); //clear littlefs

    void writeSingle(size_t index, uint16_t value); //write single register
    size_t writeMultiple(size_t startIndex, size_t buffSize, uint16_t *buff); //write multiple parameter

    uint16_t getBaudrate(); //get baudrate (0 - 6)
    int getBaudrateBps(); //get baudrate in bps value
    uint16_t getId(); //get id from flash
    uint16_t getOvervoltageDisconnect1(); //get overvoltage disconnect 1 from flash
    uint16_t getOvervoltageReconnect1(); //get overvoltage reconnect 1 from flash
    uint16_t getUndervoltageDisconnect1(); //get overvoltage undervoltage 1 from flash
    uint16_t getUndervoltageReconnect1(); //get overvoltage reconnect 1 from flash
    uint16_t getOvercurrentDisconnect1(); //get overcurrent disconnect 1 from flash
    uint16_t getOvercurrentDetectionTime1(); //get overcurrent detection time 1 from flash
    uint16_t getOvercurrentReconnectInterval1(); //get overcurrent reconnect interval 1 from flash
    uint16_t getShortCircuitDisconnect1(); //get short circuit disconnect 1 from flash
    uint16_t getShortCircuitDetectionTime1(); //get short circuit detection time 1 from flash
    uint16_t getShortCircuitReconnectInterval1(); //get short circuit reconnect interval 1 from flash
    uint16_t getOutputMode1(); //get output 1 mode from flash

    uint16_t getOvervoltageDisconnect2();
    uint16_t getOvervoltageReconnect2();
    uint16_t getUndervoltageDisconnect2();
    uint16_t getUndervoltageReconnect2();
    uint16_t getOvercurrentDisconnect2();
    uint16_t getOvercurrentDetectionTime2();
    uint16_t getOvercurrentReconnectInterval2();
    uint16_t getShortCircuitDisconnect2();
    uint16_t getShortCircuitDetectionTime2();
    uint16_t getShortCircuitReconnectInterval2();
    uint16_t getOutputMode2();

    uint16_t getOvervoltageDisconnect3();
    uint16_t getOvervoltageReconnect3();
    uint16_t getUndervoltageDisconnect3();
    uint16_t getUndervoltageReconnect3();
    uint16_t getOvercurrentDisconnect3();
    uint16_t getOvercurrentDetectionTime3();
    uint16_t getOvercurrentReconnectInterval3();
    uint16_t getShortCircuitDisconnect3();
    uint16_t getShortCircuitDetectionTime3();
    uint16_t getShortCircuitReconnectInterval3();
    uint16_t getOutputMode3();

    size_t getAllParameter(loadParamRegister &regs); //get all stored parameter

    ~LoadParameter();
};




#endif