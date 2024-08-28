#ifndef MODBUS_OTA_H
#define MODBUS_OTA_H

#define OTA_BUFFER_SIZE 200

#include <Arduino.h>

namespace MbusOTA {
    enum Address : uint16_t {
        START_FW_OTA = 0x0001,
        START_FLASH_OTA,
        TRANSMIT_OTA,
        END_OTA,
        START_COMPRESSED_FW_OTA,
        START_COMPRESSED_FLASH_OTA,
        TRANSMIT_COMPRESSED_OTA,
        END_COMPRESSED_OTA
    };

    enum FlagStatus : uint8_t {
        WAIT,
        START_FW,
        START_FLASH,
        TRANSMIT,
        END,
        START_FW_COMPRESSED,
        START_FLASH_COMPRESSED,
        TRANSMIT_COMPRESSED,
        END_COMPRESSED
    };

    struct OtaData
    {
        uint8_t data[OTA_BUFFER_SIZE];
        size_t currentSize = 0;
        uint8_t status = FlagStatus::WAIT;
    };
};

class ModbusOta
{
private:
    /* data */
public:
    ModbusOta(/* args */);
    ~ModbusOta();
};


#endif