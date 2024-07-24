#include "Utilities.h"

int Utilities::uint16ArrayToCharArray(uint16_t *data, size_t dataLength, char *buff, size_t buffLength, bool swap)
{
    bool isLessCharacter = true;
    if (dataLength * 2 >= buffLength)
    {
        // ESP_LOGI(_TAG, "Character buffer ovf\n");
        return -1;
    }
    for (size_t i = 0; i < dataLength; i++)
    {
        uint16_t tBuff = data[i];
        if (swap)
        {
            tBuff = Utilities::swap16(data[i]);
        }
        char msb = tBuff >> 8;
        char lsb = tBuff & 0xFF;
        buff[i*2] = msb;
        buff[i*2+1] = lsb;
    }
    for (size_t i = dataLength*2; i < buffLength; i++)
    {
        buff[i] = 0;
    }
    return 1;
}

/**
 * Convert string into double char (16bit), e.g hello will translate into "he", "ll" "o\0"
 * 
 * @param[in]   s   string to be converted into
 * @param[in]   *buff   pointer of array
 * @param[in]   length  the length of the buff array
 * @param[in]   swap    swap the character order
 * @return  the length of the converted array
*/
size_t Utilities::toDoubleChar(String s, uint16_t *buff, size_t length, bool swap)
{
    size_t stringLength = s.length();
    // Serial.println("string length : " + String(stringLength));
    size_t resultLength = 0;
    bool isEven = false;
    bool isLessCharacter = false;

    // check the length of string, make sure the buffer is big enough to store
    if (stringLength > length*2)
    {
        return resultLength;
    }
    else
    {
        isLessCharacter = true;
    }

    // check even or odd character count
    if (stringLength % 2 == 0)
    {
        // Serial.println("Even");
        isEven = true;
        resultLength = stringLength / 2;
    }
    else
    {
        // Serial.println("Odd");
        resultLength = (stringLength / 2) + 1;
    }

    // fill the buff array with 0
    for (size_t i = 0; i < length; i++)
    {
        buff[i] = 0;
    }

    for (size_t i = 0; i < resultLength; i++)
    {
        if (isEven)
        {
            if (swap)
            {
                buff[i] = Utilities::charConcat(s.charAt(i*2 + 1), s.charAt(i*2)); //place higher index to leftmost character
            }
            else
            {
                buff[i] = Utilities::charConcat(s.charAt(i*2), s.charAt(i*2 + 1)); //place lower index to leftmost character
            }
            
            if (isLessCharacter) // if less than 32 character, fill the leftover with null
            {
                for (int j = resultLength; j < length; j++)
                {
                    buff[j] = 0; //add null terminator
                }
            }
        }
        else
        {
            // only executed if the character is odd, 
            if (i == (resultLength - 1))
            {
                if (swap)
                {
                    buff[i] = Utilities::charConcat('\0', s.charAt(i*2)); // add null terminator at the very end
                }
                else
                {
                    buff[i] = Utilities::charConcat(s.charAt(i*2), '\0'); // add null terminator at the very end
                }
                    
                
                if (isLessCharacter) // if less than 32 character, fill the leftover with null
                {
                    for (int j = resultLength; j < length; j++)
                    {
                        buff[j] = 0; //Null terminator
                    }
                }
            }
            else
            {
                if (swap)
                {
                    buff[i] = Utilities::charConcat(s.charAt(i*2 + 1), s.charAt(i*2)); //place higher index to leftmost character
                }
                else
                {
                    buff[i] = Utilities::charConcat(s.charAt(i*2), s.charAt(i*2 + 1)); //place lower index to leftmost character
                }            
            }
        }
    }    
    return resultLength; 
}

/**
 * Concatenate two char (uint8) into 16 bit unsigned int
 * 
 * @param[in]   first   the first character
 * @param[in]   second  the second character
 * @return  result of the 2 char concatenated
*/
uint16_t Utilities::charConcat(const char &first, const char &second)
{
    uint16_t result = 0;

    result = (first << 8) + second;

    return result;
}

/**
 * Swap the MSB and LSB
 * 
 * @param[in]   value   the value to be swapped
 * @return  swapped value
*/
uint16_t Utilities::swap16(uint16_t value)
{
    uint16_t result = 0;

    uint8_t first = value >> 8;
    uint8_t second = value & 0xff;

    result = (second << 8) + first;
    return result;
}

/**
 * Get bit from specified position
 * 
 * @param[in]   pos position of the bit
 * @param[in]   data    data that need to be extracted
 * @return  bit value, 0 or 1
*/
int Utilities::getBit(int pos, int data)
{
  if (pos > 7 & pos < 0)
  {
    return -1;
  }
  int temp = data >> pos;
  int result = temp & 0x01;
  return result;
}

