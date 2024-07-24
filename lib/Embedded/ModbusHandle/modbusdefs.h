#ifndef MODBUS_HANDLE_H
#define MODBUS_HANDLE_H

#include "array"

namespace Modbus_Defs {
    struct modbusRegister
    {
        std::array<uint16_t, 10> inputRegister;
        std::array<uint16_t, 23> holdingRegister;

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
    };
    
}

#endif