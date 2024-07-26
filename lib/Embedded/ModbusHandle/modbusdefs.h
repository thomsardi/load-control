#ifndef MODBUS_HANDLE_H
#define MODBUS_HANDLE_H

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

#endif