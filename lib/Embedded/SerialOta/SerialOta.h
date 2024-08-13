#ifndef SERIAL_OTA_H
#define SERIAL_OTA_H

#define OTA_BUFFER_SIZE 64

#include <array>
#include <vector>
#include <ArduinoJson.h>

enum FlagStatus : uint8_t {
    WAIT,
    START_FW,
    START_FLASH,
    PROCESS,
    START_FW_COMPRESSED,
    START_FLASH_COMPRESSED,
    PROCESS_COMPRESSED,
    END,
    END_COMPRESSED
};

enum ResponseStatus : uint8_t {
    IDLE,
    ACK_FW,
    ACK_FLASH,
    RECEIVED,
    FAILED,
    FINISHED,
    RETRY,
    ERROR
};

enum CmdCode : uint8_t {
    UPLOAD_FW = 0x11,
    UPLOAD_FLASH = 0x12,
    TRANSMIT = 0x13,
    END_TRANSMIT = 0x14,
    UPLOAD_FW_COMPRESSED = 0x15,
    UPLOAD_FLASH_COMPRESSED = 0X16,
    TRANSMIT_COMPRESSED = 0X17,
    END_TRANSMIT_COMPRESSED = 0X18,
};

enum MemoryAddress : uint8_t {
    FW_ADDRESS = 0x01,
    FLASH_ADDRESS = 0x02
};

enum ExceptionCode : uint8_t {
    ILLEGAL_FUNCTION = 0x01,
    ILLEGAL_ADDRESS = 0x02,
    SLAVE_BUSY = 0x06
};

struct OtaData
{
    uint8_t data[OTA_BUFFER_SIZE];
    size_t currentSize = 0;
    uint8_t status = FlagStatus::WAIT;
};

struct Response
{
    uint8_t responseStatus = ResponseStatus::IDLE;
    std::vector<uint8_t> responseData;

    Response()
    {
        responseData.reserve(8);
    }
};

class SerialUploader {
    public:
        SerialUploader();
        void begin(uint8_t id);
        Response process(uint8_t buff[], size_t len);
        std::vector<OtaData> _otaDataQueue;
        ~SerialUploader();
    private:
        const char* _TAG = "Serial Uploader";
        uint8_t _id = 1;
        uint16_t calculateCrc16(uint8_t buff[], size_t len);
        uint16_t getCrc16(uint8_t buff[], size_t len);
        uint8_t getSlaveId(uint8_t buff[], size_t len);
        uint8_t getCmd(uint8_t buff[], size_t len);
        uint16_t get16(int offset, uint8_t buff[], size_t len);
        uint8_t get8(int offset, uint8_t buff[], size_t len);
        void buildException(uint8_t exceptionCode, std::vector<uint8_t> &buffResponse);
};

#endif