#include "cc6940.h"

CC6940::CC6940(/* args */)
{
}

/**
 * Setup the class
 * 
 * @param[in]   config  config parameter, refer to CC6940Config struct
 */
void CC6940::setup(CC6940Config &config)
{
    _resolution = config.resolution;
    _maxVoltage = config.maxVoltage;
    _minAdcValue = config.minAdcValue;
    _maxAdcValue = config.maxAdcValue;
    _offsetMidpoint = config.offsetMidpoint;
}

/**
 * Get preset config
 * 
 * @param[in]   type    type of the device according to CC6940Type, refer to CC6940Type for more information
 * 
 * @return  CC6940Config struct
 */
CC6940Config CC6940::getPresetConfig(CC6940Type type)
{
    CC6940Config config;
    switch (type)
    {
    case CC6940Type::CURRENT_10A :
        config.maxVoltage = 3.3;
        config.minAdcValue = 0;
        config.maxAdcValue = 4096;
        config.resolution = 0.132;
        config.offsetMidpoint = 0;
        break;
    case CC6940Type::CURRENT_20A :
        config.maxVoltage = 3.3;
        config.minAdcValue = 0;
        config.maxAdcValue = 4096;
        config.resolution = 0.066;
        config.offsetMidpoint = 0;
        break;
    case CC6940Type::CURRENT_30A :
        config.maxVoltage = 3.3;
        config.minAdcValue = 0;
        config.maxAdcValue = 4096;
        config.resolution = 0.044;
        config.offsetMidpoint = 0;
        break;
    default:
        break;
    }
    return config;
}

/**
 * Get active config
 * 
 * @return CC6940Config struct
 */
CC6940Config CC6940::getCurrentConfig()
{
    CC6940Config config;
    config.maxVoltage = _maxVoltage;
    config.minAdcValue = _minAdcValue;
    config.maxAdcValue = _maxAdcValue;
    config.resolution = _resolution;
    config.offsetMidpoint = _offsetMidpoint;
    return config;
}

/**
 * Convert from raw adc value into current based on the config
 * 
 * @param[in]   adcValue    unsigned raw adc value
 * 
 * @return  float   current in ampere
 */
float CC6940::getCurrent(uint16_t adcValue)
{
    uint16_t adcRange = _maxAdcValue - _minAdcValue; //get span
    uint16_t midPoint = _minAdcValue + (adcRange / 2) + _offsetMidpoint; //get midpoint
    int valueRaw = adcValue - midPoint; //get adc raw value difference from midpoint to reading value
    // ESP_LOGI(_TAG, "raw analog : %d", valueRaw);
    float voltage = ((float)valueRaw / adcRange) * _maxVoltage; //convert adc value into voltage
    float result = voltage / _resolution; // get current by dividing voltage with resolution
    // ESP_LOGI(_TAG, "result : %.2f", result);
    return result;
}

CC6940::~CC6940()
{
}