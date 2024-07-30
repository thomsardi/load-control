#ifndef LOAD_HANDLE_H
#define LOAD_HANDLE_H

#include <Arduino.h>
#include "array"

namespace LoadModbus {
    struct modbusRegister
    {
        std::array<uint16_t, 10> inputRegister;
        std::array<uint16_t, 23> holdingRegister;

        modbusRegister()
        {
            inputRegister.fill(0);
            holdingRegister.fill(0);
        }

        void assignLoadVoltage1(uint16_t value)
        {
            inputRegister[0] = value;
        }

        void assignLoadVoltage2(uint16_t value)
        {
            inputRegister[1] = value;
        }

        void assignLoadVoltage3(uint16_t value)
        {
            inputRegister[2] = value;
        }

        void assignSystemVoltage(uint16_t value)
        {
            inputRegister[3] = value;
        }

        void assignLoadCurrent1(uint16_t value)
        {
            inputRegister[4] = value;
        }

        void assignLoadCurrent2(uint16_t value)
        {
            inputRegister[5] = value;
        }

        void assignLoadCurrent3(uint16_t value)
        {
            inputRegister[6] = value;
        }

        void assignFlag1(uint16_t value)
        {
            inputRegister[7] = value;
        }

        void assignFlag2(uint16_t value)
        {
            inputRegister[8] = value;
        }

        void assignFlag3(uint16_t value)
        {
            inputRegister[9] = value;
        }

        size_t assignHoldingRegister(std::array<uint16_t, 23> &regs)
        {
            size_t regsNumber = 0;
            for (size_t i = 0; i < holdingRegister.size(); i++)
            {
                holdingRegister[i] = regs[i];
                regsNumber++;
            }
            return regsNumber;
        }
    };
}

/**
 * Load parameter struct
 * 
 * voltage in 0.1V
 * current in 0.01A
 * time in miliseconds (ms)
 * 
 * activeLow, true to set it as sink (provide return / ground path), false to set it as source (provide power path)
 */
struct LoadParamsSetting {
    uint16_t loadOverVoltageDisconnectVoltage = 600; // voltage in 0.1V
    uint16_t loadOvervoltageReconnectVoltage = 580;
    uint16_t loadUndervoltageDisconnectVoltage = 500;
    uint16_t loadUndervoltageReconnectVoltage = 510;
    uint16_t loadOvercurrent = 1000;    // current int 0.01A
    uint16_t loadOcDetectionTime = 4000;    // wait time in miliseconds (ms)
    uint16_t loadOcReconnectTime = 4000;    // reconnect time in miliseconds (ms)
    bool activeLow = false; //set to true if sink (low side switch), set false if source (high side switch)
};

/**
 * bitfield
 * 
 * format for bitfield
 * 
 * bit 0 = overvoltage bit
 * bit 1 = undervoltage bit
 * bit 2 = overcurrent bit
 * bit 3 - 15 = reserved for future use
 */
union bitField {
    struct flagStatus
    {
        uint16_t overvoltage : 1;
        uint16_t undervoltage : 1;
        uint16_t overcurrent : 1;
        uint16_t : 13;
    } flag;
    uint16_t value;
};

class LoadHandle {
    private :
        uint16_t _loadOvervoltageDisconnectVoltage;
        uint16_t _loadOvervoltageReconnectVoltage;
        uint16_t _loadUndervoltageDisconnectVoltage;
        uint16_t _loadUndervoltageReconnectVoltage;
        uint16_t _loadOvercurrent;
        uint16_t _loadOcDetectionTime;
        uint16_t _loadOcReconnectTime;
        bitField _bitStatus;
        unsigned long _lastOcCheck;
        unsigned long _lastOcReconnect;
        bool _isActiveLow;
        bool _state;

    public :
        LoadHandle();
        void setParams(const LoadParamsSetting &load_params_t);
        void loop(uint16_t loadVoltage, uint16_t loadCurrent);
        bool getAction();
        bool isOvervoltage();
        bool isUndervoltage();
        bool isOvercurrent();
        uint16_t getStatus();
        ~LoadHandle();
};

#endif