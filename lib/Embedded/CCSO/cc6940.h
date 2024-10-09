#ifndef CC6940_H
#define CC6940_H

#include <Arduino.h>
#include <stdint.h>

enum CC6940Type : uint8_t {
    CURRENT_10A,
    CURRENT_20A,
    CURRENT_30A
};

struct CC6940Config {
    uint32_t midPoint = 1650;
    int32_t offset = 0;
    float multiplier = 1;
    uint32_t resolution = 132; // 132mV / A, for every 132mV equal to 1 Ampere
};

class CC6940
{
private:
    /* data */
    const char* _TAG = "cc6940-class";
    uint32_t _midPoint = 1650;
    int32_t _offset = 0;
    float _multiplier = 1;
    uint32_t _resolution = 132;
public:
    CC6940();
    void setup(CC6940Config &config);   //setup the parameter for class
    CC6940Config getCurrentConfig();    //get current config of class
    CC6940Config getPresetConfig(CC6940Type type);  //get preset config
    float getCurrent(uint16_t adcValue);    //convert raw adc value into current
    float getCurrent(uint32_t adcInMillivolts);
    ~CC6940();
};



#endif