#ifndef CC6940_H
#define CC6940_H

#include <stdint.h>

enum CC6940Type : uint8_t {
    CURRENT_10A,
    CURRENT_20A,
    CURRENT_30A
};

struct CC6940Config {
    float maxVoltage = 3.3; //3.3V
    uint16_t maxAdcValue = 4096;    //12 bit resolution 2^12 = 4096
    uint16_t minAdcValue = 0;   //lowest value (offset)
    float resolution = 0.132; // 132mV / A, for every 132mV equal to 1 Ampere
    int16_t offsetMidpoint = 0;
};

class CC6940
{
private:
    /* data */
    const char* _TAG = "cc6940-class";
    float _resolution = 0.132;
    float _maxVoltage = 3.3;
    uint16_t _maxAdcValue = 4096;
    uint16_t _minAdcValue = 0;
    uint16_t _offsetMidpoint = 0;
public:
    CC6940();
    void setup(CC6940Config &config);   //setup the parameter for class
    CC6940Config getCurrentConfig();    //get current config of class
    CC6940Config getPresetConfig(CC6940Type type);  //get preset config
    float getCurrent(uint16_t adcValue);    //convert raw adc value into current
    ~CC6940();
};



#endif