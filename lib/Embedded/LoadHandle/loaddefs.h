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

    union FeedbackStatus {
        struct bitField {
            uint16_t mcb1 : 1;
            uint16_t mcb2 : 1;
            uint16_t mcb3 : 1;
            uint16_t relayFeedback1 : 1;
            uint16_t relayFeedback2 : 1;
            uint16_t relayFeedback3 : 1;
            uint16_t relayOnFailed1: 1;
            uint16_t relayOffFailed1: 1;
            uint16_t relayOnFailed2: 1;
            uint16_t relayOffFailed2: 1;
            uint16_t relayOnFailed3: 1;
            uint16_t relayOffFailed3: 1;
            uint16_t :4;
        } flag;
        uint16_t value;
    };
    
    struct modbusRegister
    {
        std::array<uint16_t, 12> inputRegister;
        std::array<uint16_t, 35> holdingRegister;

        modbusRegister()
        {
            inputRegister.fill(0);
            holdingRegister.fill(0);
        }

        void assignLoadVoltage1(int16_t value)
        {
            inputRegister[0] = value;
        }

        void assignLoadVoltage2(int16_t value)
        {
            inputRegister[1] = value;
        }

        void assignLoadVoltage3(int16_t value)
        {
            inputRegister[2] = value;
        }

        void assignSystemVoltage(int16_t value)
        {
            inputRegister[3] = value;
        }

        void assignLoadCurrent1(int16_t value)
        {
            inputRegister[4] = value;
        }

        void assignLoadCurrent2(int16_t value)
        {
            inputRegister[5] = value;
        }

        void assignLoadCurrent3(int16_t value)
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

        void assignFeedbackStatus(uint16_t value)
        {
            inputRegister[10] = value;
        }

        void assignSystemStatus(uint16_t value)
        {
            inputRegister[11] = value;
        }

        size_t assignHoldingRegister(std::array<uint16_t, 35> &regs)
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
    uint16_t loadOvercurrentDisconnect = 1000;    // overcurrent in 0.01A
    uint16_t loadOcDetectionTime = 2000;    // wait time in miliseconds (ms)
    uint16_t loadOcReconnectTime = 4000;    // reconnect time in miliseconds (ms)
    uint16_t loadShortCircuitDisconnect = 2000;    // short circuit current in 0.01A
    uint16_t loadShortCircuitDetectionTime = 20;    // wait time in miliseconds (ms)
    uint16_t loadShortCircuitReconnectTime = 4000;    // reconnect time in miliseconds (ms)
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
        uint16_t undervoltage : 1;
        uint16_t overvoltage : 1;
        uint16_t overcurrent : 1;
        uint16_t shortCircuit : 1;
        uint16_t : 12;
    } flag;
    uint16_t value;
};

class LoadHandle {
    private :
        const char* _TAG = "load-handle";
        
        uint16_t _loadOvervoltageDisconnect;
        uint16_t _loadOvervoltageReconnect;
        uint16_t _loadUndervoltageDisconnect;
        uint16_t _loadUndervoltageReconnect;
        uint16_t _loadOvercurrentDisconnect;
        uint16_t _loadOcDetectionTime;
        uint16_t _loadOcReconnectTime;
        uint16_t _loadShortCircuitDisconnect;
        uint16_t _loadShortCircuitDetectionTime;
        uint16_t _loadShortCircuitReconnectTime;
        bitField _bitStatus;
        unsigned long _lastOcCheck;
        unsigned long _lastOcReconnect;
        unsigned long _lastScCheck;
        unsigned long _lastScReconnect;
        bool _isActiveLow;
        bool _state;

    public :
        LoadHandle();
        void setParams(const LoadParamsSetting &load_params_t);
        void printParams();
        void loop(int16_t loadVoltage, int16_t loadCurrent);
        bool getAction();
        bool isOvervoltage();
        bool isUndervoltage();
        bool isOvercurrent();
        bool isShortCircuit();
        float toCurrent(int raw, int gain = 66, int maxRaw = 4096, int minRaw = 0, int midPoint = 2048);
        uint16_t getStatus();
        ~LoadHandle();
};

#endif