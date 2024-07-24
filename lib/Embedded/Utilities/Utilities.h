#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

class Utilities {
    public :
        static int uint16ArrayToCharArray(uint16_t *data, size_t dataLength, char *buff, size_t buffLength, bool swap);
        static size_t toDoubleChar(String s, uint16_t *buff, size_t length, bool swap = false);
        static uint16_t charConcat(const char &first, const char &second);
        static uint16_t swap16(uint16_t value);
        static int getBit(int pos, int data);
        template <typename T> static bool _getBit(int pos, T data);
        template <typename T> static void fillArray(T a[], size_t len, T value);
        template <typename T> static void fillArrayRandom(T a[], size_t len, T min, T max);
    private :
        const char* _TAG = "Utilities class";

};

/**
 * Not yet implemented, do not call this method
 * @brief   extract bit from specified data type
 * @param[in]   pos position of bit
 * @tparam  data    data value to be extracted
 * @return  extracted bit value (1 or 0) and always 0 if error
 * 
*/
template <typename T>
bool Utilities::_getBit(int pos, T data)
{
  size_t bitsCount = sizeof(data) * 8;
  Serial.println("Bits count : " + String(bitsCount));
  if (pos >= bitsCount & pos < 0) // prevent to access out of bound
  {
    return 0;
  }
  int temp = data >> pos;
  bool result = temp & 0x01;
  return result;
};

/**
 * Template class to fill array with specified value
 * 
 * @brief       fill array with specified value
 * @tparam      a[] the data array to be filled
 * @param[in]   len length of the array
 * @tparam      value   value to fill the array  
*/
template <typename T>
void Utilities::fillArray(T a[], size_t len, T value)
{
    for (size_t i = 0; i < len; i++)
    {
        a[i] = value;
    }
};

/**
 * Template class to fill array with random value between min and max
 * 
 * @brief       fill array with random value
 * @tparam      a[] the data array to be filled
 * @param[in]   len length of the array
 * @tparam      min minimum value
 * @tparam      max maximum value  
*/
template <typename T>
void Utilities::fillArrayRandom(T a[], size_t len, T min, T max)
{
    for (size_t i = 0; i < len; i++)
    {
        a[i] = random(min, max);
    }
};

#endif