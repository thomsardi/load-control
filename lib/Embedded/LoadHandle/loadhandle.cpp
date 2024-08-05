#include "loaddefs.h"

LoadHandle::LoadHandle()
{
    
}

/**
 * Set load parameter
 * 
 * @param[in]   LoadParamsSetting struct, voltage value is in 0.1V (505 means 50.5V,), current value is in 0.01A (1050 means 10.5A), time in miliseconds (4000 means 4s)
 */
void LoadHandle::setParams(const LoadParamsSetting &load_params_t)
{
    _loadOvervoltageDisconnect = load_params_t.loadOverVoltageDisconnect;
    _loadOvervoltageReconnect = load_params_t.loadOvervoltageReconnect;
    _loadUndervoltageDisconnect = load_params_t.loadUndervoltageDisconnect;
    _loadUndervoltageReconnect = load_params_t.loadUndervoltageReconnect;
    _loadOvercurrentDisconnect = load_params_t.loadOvercurrentDisconnect;
    _loadOcDetectionTime = load_params_t.loadOcDetectionTime;
    _loadOcReconnectTime = load_params_t.loadOcReconnectTime;
    _isActiveLow = load_params_t.activeLow;
}

/**
 * Main loop
 * @brief   call this method periodically to handle load according to voltage and current
 * 
 * @param[in]   loadVoltage load voltage in 0.1V
 * @param[in]   loadCurrent load current in 0.01A
 */
void LoadHandle::loop(uint16_t loadVoltage, uint16_t loadCurrent)
{
    if (!_bitStatus.flag.overvoltage && !_bitStatus.flag.undervoltage && !_bitStatus.flag.overcurrent)
    {
        _state = !_isActiveLow;
    }
    else
    {
        _state = _isActiveLow;
    }

    if (loadVoltage > _loadOvervoltageDisconnect)
    {
        _bitStatus.flag.overvoltage = 1;
    }

    if (loadVoltage < _loadOvervoltageReconnect)
    {
        _bitStatus.flag.overvoltage = 0;
    }

    if (loadVoltage < _loadUndervoltageDisconnect)
    {
        _bitStatus.flag.undervoltage = 1;
    }

    if (loadVoltage > _loadUndervoltageReconnect)
    {
        _bitStatus.flag.undervoltage = 0;
    }

    if (loadCurrent > _loadOvercurrentDisconnect && !_bitStatus.flag.overcurrent)
    {
        if (millis() - _lastOcCheck > _loadOcDetectionTime)
        {
            _bitStatus.flag.overcurrent = 1;
        }
    }
    else
    {
        _lastOcCheck = millis();
    }

    if (_bitStatus.flag.overcurrent)
    {
        if (millis() - _lastOcReconnect > _loadOcReconnectTime)
        {
            _bitStatus.flag.overcurrent = 0;
        }
    }
    else
    {
        _lastOcReconnect = millis();
    }
}

/**
 * Get action state
 * 
 * @return  state of action, LOW or HIGH
 */
bool LoadHandle::getAction()
{
    return _state;
}

/**
 * Get overvoltage status
 * 
 * @return  overvoltage flag bit
 */
bool LoadHandle::isOvervoltage()
{
    return _bitStatus.flag.overvoltage;
}

/**
 * Get undervoltage status
 * 
 * @return  undervoltage flag bit
 */
bool LoadHandle::isUndervoltage()
{
    return _bitStatus.flag.undervoltage;
}

/**
 * Get overcurrent status
 * 
 * @return overcurrent flag bit
 */
bool LoadHandle::isOvercurrent()
{
    return _bitStatus.flag.overcurrent;
}

/**
 * convert adc raw value to current
 * 
 * @param[in] raw raw value of adc
 * @param[in] gain  gain in mV/A
 * @param[in] maxRaw    maximum value of adc
 * @param[in] minRaw    minimum value of adc
 * @param[in] midPoint    middlepoint value of adc
 * 
 * @return  current value in float
 */
float LoadHandle::toCurrent(int raw, int gain, int maxRaw, int minRaw, int midPoint)
{
    float resolution = 3300; 
    resolution /= (maxRaw - minRaw); // for 1 adc value equal to how many millivolt
    float milliVolt = (raw - minRaw - midPoint) * resolution; // multiply the adc value with resolution to get actual millivolt
    float current = milliVolt / gain;   // multiply the millivolt with gain to get current
    return current;
}

/**
 * Get status flag in uint16
 * 
 * @return  all flag status bit packed in uint16
 */
uint16_t LoadHandle::getStatus()
{
    return _bitStatus.value;
}

LoadHandle::~LoadHandle()
{

}