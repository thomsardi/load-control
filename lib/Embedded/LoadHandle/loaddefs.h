#ifndef LOAD_HANDLE_H
#define LOAD_HANDLE_H

#include <Arduino.h>
#include "array"

namespace LoadModbus {
    /**
     * Bitfield system status
     * 
     * bit 0 : run status
     * bit 1 : mode status
     * bit 2 - 15 : unused
     */
    union SystemStatus {
        struct bitField {
            uint16_t run : 1;
            uint16_t mode : 1;
            uint16_t : 14;
        } flag;
        uint16_t value;
    };

    /**
     * Bitfield feedback status
     * 
     * bit 0 : mcb1 status
     * bit 1 : mcb2 status
     * bit 2 : mcb3 status
     * bit 3 : relay feedback 1 status
     * bit 4 : relay feedback 2 status
     * bit 5 : relay feedback 3 status
     * bit 6 : relay 1 ON failed
     * bit 7 : relay 1 OFF failed
     * bit 8 : relay 2 ON failed
     * bit 9 : relay 2 OFF failed
     * bit 10 : relay 3 ON failed
     * bit 11 : relay 3 OFF failed
     * bit 12 - 15 : unused
     */
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
        std::array<uint16_t, 12> inputRegister; //reserve 12 input register
        std::array<uint16_t, 35> holdingRegister; //reserve 35 holding register

        modbusRegister()
        {
            inputRegister.fill(0); //fill input register with zero value
            holdingRegister.fill(0); //fill holding register with zero value
        }

        /**
         * assign load voltage 1 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadVoltage1(int16_t value)
        {
            inputRegister[0] = value;
        }

        /**
         * assign load voltage 2 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadVoltage2(int16_t value)
        {
            inputRegister[1] = value;
        }

        /**
         * assign load voltage 3 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadVoltage3(int16_t value)
        {
            inputRegister[2] = value;
        }

        /**
         * assign system voltage register
         * @param[in]   value   value to be assigned into
         */
        void assignSystemVoltage(int16_t value)
        {
            inputRegister[3] = value;
        }

        /**
         * assign load current 1 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadCurrent1(int16_t value)
        {
            inputRegister[4] = value;
        }

        /**
         * assign load current 2 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadCurrent2(int16_t value)
        {
            inputRegister[5] = value;
        }

        /**
         * assign load current 3 register
         * @param[in]   value   value to be assigned into
         */
        void assignLoadCurrent3(int16_t value)
        {
            inputRegister[6] = value;
        }

        /**
         * assign flag 1 register
         * @param[in]   value   value to be assigned into
         */
        void assignFlag1(uint16_t value)
        {
            inputRegister[7] = value;
        }

        /**
         * assign flag 2 register
         * @param[in]   value   value to be assigned into
         */
        void assignFlag2(uint16_t value)
        {
            inputRegister[8] = value;
        }

        /**
         * assign flag 3 register
         * @param[in]   value   value to be assigned into
         */
        void assignFlag3(uint16_t value)
        {
            inputRegister[9] = value;
        }

        /**
         * assign feedback status register
         * @param[in]   value   value to be assigned into
         */
        void assignFeedbackStatus(uint16_t value)
        {
            inputRegister[10] = value;
        }

        /**
         * assign system status register
         * @param[in]   value   value to be assigned into
         */
        void assignSystemStatus(uint16_t value)
        {
            inputRegister[11] = value;
        }

        /**
         * assign holding register
         * @param[in]   regs    array of 35 element
         * 
         * @return  number of written register
         */
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
 * bitfield flag status
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
        void setParams(const LoadParamsSetting &load_params_t); //set object parameter
        void printParams(); //print parameter stored
        void loop(int16_t loadVoltage, int16_t loadCurrent); //main loop
        bool getAction(); //get action
        bool isOvervoltage(); //get overvoltage flag
        bool isUndervoltage(); //get undervoltage flag
        bool isOvercurrent(); //get overcurrent flag
        bool isShortCircuit(); //get short circuit flag
        uint16_t getStatus(); //get all flag status as uint16
        ~LoadHandle();
};

#endif