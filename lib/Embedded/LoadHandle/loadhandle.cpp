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
    _loadOvervoltageDisconnectVoltage = load_params_t.loadOverVoltageDisconnectVoltage;
    _loadOvervoltageReconnectVoltage = load_params_t.loadOvervoltageReconnectVoltage;
    _loadUndervoltageDisconnectVoltage = load_params_t.loadUndervoltageDisconnectVoltage;
    _loadUndervoltageReconnectVoltage = load_params_t.loadUndervoltageReconnectVoltage;
    _loadOvercurrent = load_params_t.loadOvercurrent;
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

    if (loadVoltage > _loadOvervoltageDisconnectVoltage)
    {
        _bitStatus.flag.overvoltage = 1;
    }

    if (loadVoltage < _loadOvervoltageReconnectVoltage)
    {
        _bitStatus.flag.overvoltage = 0;
    }

    if (loadVoltage < _loadUndervoltageDisconnectVoltage)
    {
        _bitStatus.flag.undervoltage = 1;
    }

    if (loadVoltage > _loadUndervoltageReconnectVoltage)
    {
        _bitStatus.flag.undervoltage = 0;
    }

    if (loadCurrent > _loadOvercurrent && !_bitStatus.flag.overcurrent)
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