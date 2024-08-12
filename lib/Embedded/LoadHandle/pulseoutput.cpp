#include "pulseoutput.h"

PulseOutput::PulseOutput()
{
    
}

void PulseOutput::setup(uint8_t pin, int pulseOnDuration, int pulseOffDuration, bool activeLow)
{
    _pin = pin;
    _pulseOnDuration = pulseOnDuration;
    _pulseOffDuration = pulseOffDuration;
    _activeLow = activeLow;
    pinMode(_pin, OUTPUT);
}

void PulseOutput::set()
{
    if (!_isSet)
    {
        _lastPulseOnCheck = millis();
        _isSet = true;
    }
}

void PulseOutput::reset()
{
    _isSet = false;
    digitalWrite(_pin, _activeLow);
}

void PulseOutput::changeActiveState(bool activeLow)
{
    _activeLow = activeLow;
}

void PulseOutput::changePulseOnDuration(int duration)
{
    _pulseOnDuration = duration;
}

void PulseOutput::changePulseOffDuration(int duration)
{
    _pulseOffDuration = duration;
}

void PulseOutput::tick()
{
    if (_isSet)
    {
        if (millis() - _lastPulseOnCheck < _pulseOnDuration)
        {
            ESP_LOGI(_TAG, "pulse on");
            digitalWrite(_pin, !_activeLow);
            _lastPulseOffCheck = millis();
        }
        else
        {
            ESP_LOGI(_TAG, "pulse off");
            digitalWrite(_pin, _activeLow);
            if (millis() - _lastPulseOffCheck > _pulseOffDuration)
            {
                _lastPulseOnCheck = millis();
                _isSet = false;
            }
        }
    }
}

bool PulseOutput::isRunning()
{
    return _isSet;
}