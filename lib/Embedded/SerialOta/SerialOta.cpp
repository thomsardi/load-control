#include "SerialOta.h"

SerialUploader::SerialUploader()
{
    _otaDataQueue.reserve(4);
}

/**
 * begin class and assign and id
 * 
 * @param[in]   id  device id 1 - 255
*/
void SerialUploader::begin(uint8_t id)
{
    _id = id;
}

/**
 * process incoming data
 * @brief   incoming data will be processed, process incoming data into id, command, address, number of register, data length in bytes, and crc
 * 
 * @param[in]   buff    array of data
 * @param[in]   len     length of the array
 * 
 * @return  Response    refer to Response struct at SerialOta.h
*/
Response SerialUploader::process(uint8_t buff[], size_t len)
{
    Response response;

    /**
     * check the length of data, when data is below 12 bytes which mean it is broken data, drop the data. Also when incoming data is larger than
     * predefined OTA_BUFFER_SIZE + 10 bytes, drop the data. set the response status ERROR
    */
    if (len < 12 || len > (OTA_BUFFER_SIZE + 10))
    {
        response.responseStatus = ResponseStatus::ERROR;
        return response;
    }

    uint8_t slaveId = getSlaveId(buff, len);
    uint8_t cmd = getCmd(buff, len);
    uint16_t addr = get16(2, buff, len);
    uint16_t numReg = get16(4, buff, len);
    uint16_t dataLength = get16(6, buff, len);
    uint16_t crc = getCrc16(buff, len);

    /**
     * create vector of uint8_t for response message
    */
    std::vector<uint8_t> buffResponse;
    buffResponse.reserve(8);
    buffResponse.push_back(slaveId);
    buffResponse.push_back(cmd);
    
    /**
     * check for slave id
    */
    if (slaveId != _id)
    {
        ESP_LOGI(_TAG, "error slave");
        response.responseStatus = ResponseStatus::IDLE;
        return response;
    }

    /**
     * check for command, will return error when command is not recognized and illegal function exception
    */
    if ((cmd != CmdCode::UPLOAD_FLASH) && (cmd != CmdCode::UPLOAD_FW) && (cmd != CmdCode::TRANSMIT) && (cmd != CmdCode::END_TRANSMIT)
        && cmd != CmdCode::UPLOAD_FLASH_COMPRESSED && cmd != CmdCode::UPLOAD_FW_COMPRESSED && cmd != CmdCode::TRANSMIT_COMPRESSED && cmd != CmdCode::END_TRANSMIT_COMPRESSED)
    {
        ESP_LOGI(_TAG, "error cmd");
        response.responseStatus = ResponseStatus::ERROR;
        buildException(ExceptionCode::ILLEGAL_FUNCTION, buffResponse);
        for (size_t i = 0; i < buffResponse.size(); i++)
        {
            response.responseData.push_back(buffResponse.at(i));
        }
        return response;
    }
    
    /**
     * check crc16 for data integrity, when crc is not as expected, drop the data and send slave busy
    */
    if (crc != calculateCrc16(buff, len - 2))
    {
        ESP_LOGI(_TAG, "error crc");
        response.responseStatus = ResponseStatus::RETRY;
        buildException(ExceptionCode::SLAVE_BUSY, buffResponse);
        for (size_t i = 0; i < buffResponse.size(); i++)
        {
            response.responseData.push_back(buffResponse.at(i));
        }
        return response;
    }

    OtaData otaDataTemp;
    /**
     * check for number of element in ota data queue
     * when it is above or equal to 4 element, send slave busy response
    */
    if (_otaDataQueue.size() >= 4)
    {
        response.responseStatus = ResponseStatus::RETRY;
        buildException(ExceptionCode::SLAVE_BUSY, buffResponse);
        for (size_t i = 0; i < buffResponse.size(); i++)
        {
            response.responseData.push_back(buffResponse.at(i));
        }
        return response;
    }

    /**
     * process the data based on command
    */
    switch (cmd)
    {
    case CmdCode::UPLOAD_FW: //check UPLOAD_FW command
        /**
         * check for valid firmware address which is 0x0001, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::START_FW; //flag to begin firmware ota
        response.responseStatus = ResponseStatus::ACK_FW; //flag firmware acknowledge
        break;

    case CmdCode::UPLOAD_FW_COMPRESSED: //check UPLOAD_FW_COMPRESSED command
        /**
         * check for valid firmware address which is 0x0001, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::START_FW_COMPRESSED; //flag to begin firmware ota
        response.responseStatus = ResponseStatus::ACK_FW; //flag firmware acknowledge
        break;
    case CmdCode::UPLOAD_FLASH: //check UPLOAD_FLASH command
        /**
         * check for valid flash address which is 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::START_FLASH; //flag to begin flash ota
        response.responseStatus = ResponseStatus::ACK_FLASH; //flag flash acknowledge
        break;
    case CmdCode::UPLOAD_FLASH_COMPRESSED: //check UPLOAD_FLASH_COMPRESSED command
        /**
         * check for valid flash address which is 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::START_FLASH_COMPRESSED; //flag to begin flash ota
        response.responseStatus = ResponseStatus::ACK_FLASH; //flag flash acknowledge
        break;
    case CmdCode::TRANSMIT: //check TRANSMIT command
        /**
         * check for valid address which is 0x0001 or 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS && addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::PROCESS; //flag to begin flash ota
        response.responseStatus = ResponseStatus::RECEIVED; //flag data received
        break;
    case CmdCode::TRANSMIT_COMPRESSED: //check TRANSMIT_COMPRESSED command
        /**
         * check for valid address which is 0x0001 or 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS && addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::PROCESS_COMPRESSED; //flag to begin flash ota
        response.responseStatus = ResponseStatus::RECEIVED; //flag data received
        break;
    case CmdCode::END_TRANSMIT: //check END_TRANSMIT command
        /**
         * check for valid address which is 0x0001 or 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS && addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::END; //flag to end ota
        response.responseStatus = ResponseStatus::RECEIVED; //flag data received
        break;
    case CmdCode::END_TRANSMIT_COMPRESSED: //check END_TRANSMIT_COMPRESSED command
        /**
         * check for valid address which is 0x0001 or 0x0002, return illegal address when not match
        */
        if (addr != MemoryAddress::FW_ADDRESS && addr != MemoryAddress::FLASH_ADDRESS)
        {
            response.responseStatus = ResponseStatus::ERROR;
            buildException(ExceptionCode::ILLEGAL_ADDRESS, buffResponse);
            for (size_t i = 0; i < buffResponse.size(); i++)
            {
                response.responseData.push_back(buffResponse.at(i));
            }
            return response;
        }
        otaDataTemp.currentSize = dataLength;
        // copy data from buff into otaDataTemp.data, value 8 is offset to get the first data value from buff
        memcpy(otaDataTemp.data, buff + 8, dataLength);
        otaDataTemp.status = FlagStatus::END_COMPRESSED; //flag to end ota
        response.responseStatus = ResponseStatus::RECEIVED; //flag data received
        break;
    default:
        break;
    }
    _otaDataQueue.push_back(otaDataTemp); //insert into ota data queue
    /**
     * build response
    */
    buffResponse.push_back(addr >> 8);
    buffResponse.push_back(addr & 0xFF);
    buffResponse.push_back(numReg >> 8);
    buffResponse.push_back(numReg & 0xFF);
    uint16_t calCrc = calculateCrc16(buffResponse.data(), buffResponse.size());
    uint16_t hByteCrc = calCrc >> 8;
    uint16_t lByteCrc = calCrc & 0xFF;
    buffResponse.push_back(lByteCrc);
    buffResponse.push_back(hByteCrc);

    for (size_t i = 0; i < buffResponse.size(); i++)
    {
        response.responseData.push_back(buffResponse.at(i));
    }

    return response;
}

/**
 * calculate crc16 for corresponding buffer
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  crc high and low bytes are swapped
*/
uint16_t SerialUploader::calculateCrc16(uint8_t buff[], size_t len)
{
  uint16_t crc = 0xFFFF;
 
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buff[pos];          // XOR byte into least sig. byte of crc
  
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;  
}

/**
 * get slave id from buffer data
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  slave id
*/
uint8_t SerialUploader::getSlaveId(uint8_t buff[], size_t len)
{
    return buff[0];
}

/**
 * get command from buffer data
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  command
*/
uint8_t SerialUploader::getCmd(uint8_t buff[], size_t len)
{
    return buff[1];
}

/**
 * get 2 bytes (2 x 8 bit) from buffer data
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  value   uint16_t
*/
uint16_t SerialUploader::get16(int offset, uint8_t buff[], size_t len)
{
    if (offset + 1 > len)
    {
        return buff[offset];
    }
    uint16_t hByte = buff[offset];
    uint16_t lByte = buff[offset + 1];
    uint16_t result = (hByte << 8) + lByte;
    return result;
}

/**
 * get 1 byte (1 x 8 bit) from buffer data
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  value   uint8_t
*/
uint8_t SerialUploader::get8(int offset, uint8_t buff[], size_t len)
{
    if (offset > len)
    {
        return 0;
    }
    return buff[offset];
}

/**
 * get crc from buffer data
 * 
 * @param[in]   buff    array of uint8_t data
 * @param[in]   len     length of array
 * 
 * @return  crc   uint16_t with swapped bytes
*/
uint16_t SerialUploader::getCrc16(uint8_t buff[], size_t len)
{
    uint16_t lByte = buff[len-1];
    uint16_t hByte = buff[len-2];
    uint16_t crc = (lByte << 8) + hByte;
    return crc;
}

/**
 * build exception response
 * 
 * @param[in]   exceptionCode   exception code type
 * @param[in]   buffResponse    vector of uint8_t
*/
void SerialUploader::buildException(uint8_t exceptionCode, std::vector<uint8_t> &buffResponse)
{
    buffResponse.push_back(exceptionCode);
    uint16_t crc = calculateCrc16(buffResponse.data(), buffResponse.size());
    uint16_t hByte = crc >> 8;
    uint16_t lByte = crc & 0xFF;
    buffResponse.push_back(lByte);
    buffResponse.push_back(hByte);
}

SerialUploader::~SerialUploader()
{

}