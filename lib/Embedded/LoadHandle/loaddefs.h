#ifndef LOAD_HANDLE_H
#define LOAD_HANDLE_H

#include <Arduino.h>
#include "array"

namespace LoadModbus {
    union SystemStatus {
        struct bitField {
            uint16_t run : 1;
            uint16_t mode : 1;
            uint16_t : 14;
        } flag;
        uint16_t value;
    };

    union CoilStatus {
        struct bitField {
            uint16_t output1 : 1;
            uint16_t output2 : 1;
            uint16_t output3 : 1;
            uint16_t : 13;
        } flag;
        uint16_t value;
    };
    
    struct modbusRegister
    {
        std::array<uint16_t, 15> inputRegister;
        std::array<uint16_t, 26> holdingRegister;

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

        void assignLoadCurrent1(int value)
        {
            inputRegister[4] = value >> 16;
            inputRegister[5] = value & 0xFFFF;
        }

        void assignLoadCurrent2(int value)
        {
            inputRegister[6] = value >> 16;
            inputRegister[7] = value & 0xFFFF;
        }

        void assignLoadCurrent3(int value)
        {
            inputRegister[8] = value >> 16;
            inputRegister[9] = value & 0xFFFF;
        }

        void assignFlag1(uint16_t value)
        {
            inputRegister[10] = value;
        }

        void assignFlag2(uint16_t value)
        {
            inputRegister[11] = value;
        }

        void assignFlag3(uint16_t value)
        {
            inputRegister[12] = value;
        }

        void assignCoilStatus(uint16_t value)
        {
            inputRegister[13] = value;
        }

        void assignSystemStatus(uint16_t value)
        {
            inputRegister[14] = value;
        }

        size_t assignHoldingRegister(std::array<uint16_t, 26> &regs)
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
    uint16_t loadOverVoltageDisconnect = 600; // voltage in 0.1V
    uint16_t loadOvervoltageReconnect = 580;
    uint16_t loadUndervoltageDisconnect = 500;
    uint16_t loadUndervoltageReconnect = 510;
    uint16_t loadOvercurrentDisconnect = 1000;    // current int 0.01A
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
        uint16_t _loadOvervoltageDisconnect;
        uint16_t _loadOvervoltageReconnect;
        uint16_t _loadUndervoltageDisconnect;
        uint16_t _loadUndervoltageReconnect;
        uint16_t _loadOvercurrentDisconnect;
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
        float toCurrent(int raw, int gain = 66, int maxRaw = 4096, int minRaw = 0, int midPoint = 2048);
        uint16_t getStatus();
        ~LoadHandle();
};

#endif