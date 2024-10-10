#include "loaddefs.h"

LoadHandle::LoadHandle()
{
    _lastOcCheck = millis();
    _lastOcReconnect = millis();
    _lastScCheck = millis();
    _lastScReconnect = millis();
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
    _loadShortCircuitDisconnect = load_params_t.loadShortCircuitDisconnect;
    _loadShortCircuitDetectionTime = load_params_t.loadShortCircuitDetectionTime;
    _loadShortCircuitReconnectTime = load_params_t.loadShortCircuitReconnectTime;
    _isActiveLow = load_params_t.activeLow;
}

/**
 * print parameter stored
 */
void LoadHandle::printParams()
{
    ESP_LOGI(_TAG, "overvoltage disconnect : %d\n", _loadOvervoltageDisconnect);
    ESP_LOGI(_TAG, "overvoltage reconnect : %d\n", _loadOvervoltageReconnect);
    ESP_LOGI(_TAG, "undervoltage disconnect : %d\n", _loadUndervoltageDisconnect);
    ESP_LOGI(_TAG, "undervoltage reconnect : %d\n", _loadUndervoltageReconnect);
    ESP_LOGI(_TAG, "overcurrent disconnect : %d\n", _loadOvercurrentDisconnect);
    ESP_LOGI(_TAG, "overcurrent detection time : %d\n", _loadOcDetectionTime);
    ESP_LOGI(_TAG, "overcurrent reconnect time : %d\n", _loadOcReconnectTime);
    ESP_LOGI(_TAG, "short circuit disconnect : %d\n", _loadShortCircuitDisconnect);
    ESP_LOGI(_TAG, "short circuit detection time : %d\n", _loadShortCircuitDetectionTime);
    ESP_LOGI(_TAG, "short circuit reconnect time : %d\n", _loadShortCircuitReconnectTime);
    ESP_LOGI(_TAG, "output mode : %d\n", _isActiveLow);
}

/**
 * Main loop
 * @brief   call this method periodically to handle load according to voltage and current
 * 
 * @param[in]   loadVoltage load voltage in 0.1V
 * @param[in]   loadCurrent load current in 0.01A
 */
void LoadHandle::loop(int16_t loadVoltage, int16_t loadCurrent)
{
    loadCurrent = abs(loadCurrent); //make current value as absolute (always positive)

    // ESP_LOGI(_TAG, "current : %d\n", loadCurrent);
    // ESP_LOGI(_TAG, "voltage : %d\n", loadVoltage);
    
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

    /**
     * Short circuit detection
     */
    if (loadCurrent > _loadShortCircuitDisconnect && !_bitStatus.flag.shortCircuit) //check if current is above short circuit parameter and flag is not yet set(first time occured)
    {
        if (millis() - _lastScCheck > _loadShortCircuitDetectionTime) //check the duration of short circuit, if it is above parameter
        {
            // ESP_LOGI(_TAG, "===== short circuit detected =====");
            _bitStatus.flag.shortCircuit = 1; //set short circuit flag to true
            _bitStatus.flag.overcurrent = 0; //reset overcurrent flag
            _lastScCheck = millis(); //update time of last short circuit check time
            _lastOcCheck = millis(); //update time of last overcurrent check time, to prevent overcurrent detection
        }
    }
    else
    {
        _lastScCheck = millis();
    }

    if (_bitStatus.flag.shortCircuit) //if it is short circuit
    {
        if (millis() - _lastScReconnect > _loadShortCircuitReconnectTime) //check for reconnect time based on parameter
        {
            _bitStatus.flag.shortCircuit = 0; //reset short circuit flag
            _bitStatus.flag.overcurrent = 0; //reset overcurrent flag
            _lastScReconnect = millis(); //update time of last short circuit reconnect time
            _lastOcCheck = millis(); //update time of last overcurrent check, to prevent overcurrent detection
        }
    }
    else
    {
        _lastScReconnect = millis();
    }

    /**
     * Overcurrent detection
     */
    if (loadCurrent > _loadOvercurrentDisconnect && !_bitStatus.flag.overcurrent && !_bitStatus.flag.shortCircuit) //check if current is above overcurrent parameter and flag is not yet set(first time occured)
    {
        if (millis() - _lastOcCheck > _loadOcDetectionTime) //check the duration of overcurrent, if it is above parameter
        {
            // ESP_LOGI(_TAG, "===== overcurrent detected =====");
            _bitStatus.flag.overcurrent = 1; //set short circuit flag to true
            _lastOcCheck = millis(); //update time of last overcurrent check
        }
    }
    else
    {
        _lastOcCheck = millis();
    }

    if (_bitStatus.flag.overcurrent && !_bitStatus.flag.shortCircuit) //check if the flag is overcurrent and not short circuit
    {
        if (millis() - _lastOcReconnect > _loadOcReconnectTime) //check for reconnect time based on parameter
        {
            _bitStatus.flag.overcurrent = 0; //reset overcurrent flag
            _lastOcReconnect = millis(); //update time of last overcurrent reconnect time
        }
    }
    else
    {
        _lastOcReconnect = millis();
    }

    if (!_bitStatus.flag.overvoltage && !_bitStatus.flag.undervoltage && !_bitStatus.flag.overcurrent && !_bitStatus.flag.shortCircuit) //check if no flag is enabled
    {
        _state = !_isActiveLow; //return active, if activelow is enabled, this will return false otherwise return true
    }
    else
    {
        _state = _isActiveLow; //return deactivated, if activelow is enabled, this will retur true otherwise return false
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
 * Get short circuit status
 * 
 * @return short circuit flag bit
 */
bool LoadHandle::isShortCircuit()
{
    return _bitStatus.flag.shortCircuit;
}

/**
 * Get status flag in uint16
 * 
 * @return  all flag status bit packed as uint16
 */
uint16_t LoadHandle::getStatus()
{
    return _bitStatus.value;
}

LoadHandle::~LoadHandle()
{

}