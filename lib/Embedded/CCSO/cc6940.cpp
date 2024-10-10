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
    _midPoint = config.midPoint;
    _offset = config.offset;
    _multiplier = config.multiplier;
    _resolution = config.resolution;    
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
        config.midPoint = 1650; //1650mV as middle point
        config.resolution = 132; //132mV/A
        config.offset = 0; //0 mV offset between middle point
        config.multiplier = 1; // 1x multiplier
        break;
    case CC6940Type::CURRENT_20A :
        config.midPoint = 1650;
        config.resolution = 66; //66mV/A
        config.offset = 0;
        config.multiplier = 1;
        break;
    case CC6940Type::CURRENT_30A :
        config.midPoint = 1650;
        config.resolution = 44; //44mV/A
        config.offset = 0;
        config.multiplier = 1;
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
    config.midPoint = _midPoint;
    config.resolution = _resolution;
    config.multiplier = _multiplier;
    config.offset = _offset;
    return config;
}


/**
 * Convert from raw adc value into current based on the config
 * 
 * @param[in]   adcValue    unsigned raw adc value
 * 
 * @return  float   current in ampere
 */
float CC6940::getCurrent(uint32_t adcInMillivolts)
{
    // ESP_LOGI(_TAG, "adc in millivolts = %d", adcInMillivolts);
    float gapInMilivolts = (float)adcInMillivolts - (_midPoint + _offset);
    // ESP_LOGI(_TAG, "gap into midpoint = %.2f", gapInMilivolts);
    // ESP_LOGI(_TAG, "resolution = %d", _resolution);
    float result = gapInMilivolts / _resolution;
    // ESP_LOGI(_TAG, "result = %.2f", result);
    return result;
}

CC6940::~CC6940()
{
}