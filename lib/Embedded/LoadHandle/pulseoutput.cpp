#include "pulseoutput.h"

PulseOutput::PulseOutput()
{
    
}

/**
 * setup the pin, pulse duration and active mode
 * 
 * @param[in]   pin pin number
 * @param[in]   pulseOnDuration duration of ON pulse in ms
 * @param[in]   pulseOffDuration    duration of OFF pulse in ms
 * @param[in]   activeLow   active mode   
 */
void PulseOutput::setup(uint8_t pin, int pulseOnDuration, int pulseOffDuration, bool activeLow)
{
    _pin = pin;
    _pulseOnDuration = pulseOnDuration;
    _pulseOffDuration = pulseOffDuration;
    _activeLow = activeLow;
    if (_pin < 0)
    {
        return;
    }
    pinMode(_pin, OUTPUT);
}

/**
 * set single pulse
 */
void PulseOutput::set()
{
    if (!_isSet)
    {
        _lastPulseOnCheck = millis();
        _isSet = true;
    }
}

/**
 * reset the pulse
 */
void PulseOutput::reset()
{
    _isSet = false;
    if (_pin < 0)
    {
        return;
    }
    digitalWrite(_pin, _activeLow);
}

/**
 * Change active state
 * 
 * @param[in]   activeLow   true will reverse the pulse
 */
void PulseOutput::changeActiveState(bool activeLow)
{
    _activeLow = activeLow;
}

/**
 * Change pulse ON duration
 * 
 * @param[in]   duration    duration of ON pulse
 */
void PulseOutput::changePulseOnDuration(int duration)
{
    _pulseOnDuration = duration;
}

/**
 * Change pulse OFF duration
 * 
 * @param[in]   duration    duration of OFF pulse
 */
void PulseOutput::changePulseOffDuration(int duration)
{
    _pulseOffDuration = duration;
}

/**
 * main loop, call this periodically to keep track of pulse
 */
void PulseOutput::tick()
{
    if (_isSet)
    {
        if (millis() - _lastPulseOnCheck < _pulseOnDuration)
        {
            // ESP_LOGI(_TAG, "pulse on");
            if (_pin != -1)
            {
                digitalWrite(_pin, !_activeLow);
            }
            _lastPulseOffCheck = millis();
        }
        else
        {
            // ESP_LOGI(_TAG, "pulse off");
            if (_pin != -1)
            {
                digitalWrite(_pin, _activeLow);
            }
            if (millis() - _lastPulseOffCheck > _pulseOffDuration)
            {
                _lastPulseOnCheck = millis();
                _isSet = false;
            }
        }
    }
}

/**
 * Check for running status, will return true if it is in middle of process
 * 
 * @return  status of task
 */
bool PulseOutput::isRunning()
{
    return _isSet;
}