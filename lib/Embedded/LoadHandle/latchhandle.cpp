#include "latchhandle.h"

LatchHandle::LatchHandle(/* args */)
{
}

void LatchHandle::setup(int pinOn, int pinOff, int onDuration, int offDuration, bool activeLow, int interval)
{
    _pinOn = pinOn;
    _pinOff = pinOff;
    _onDuration = onDuration;
    _offDuration = offDuration;
    _activeLow = activeLow;
    _interval = interval;
    _pulseOn.setup(_pinOn, _onDuration, _offDuration, _activeLow);
    _pulseOff.setup(_pinOff, _onDuration, _offDuration, _activeLow);
}

void LatchHandle::setDuration(int onDuration, int offDuration)
{
    _onDuration = onDuration;
    _offDuration = offDuration;
    _pulseOn.changePulseOnDuration(_onDuration);
    _pulseOn.changePulseOffDuration(_offDuration);
    _pulseOff.changePulseOnDuration(_onDuration);
    _pulseOff.changePulseOffDuration(_offDuration);
}

void LatchHandle::setActiveState(bool activeLow)
{
    _activeLow = activeLow;
    _pulseOn.changeActiveState(_activeLow);
    _pulseOff.changeActiveState(_activeLow);
}

void LatchHandle::handle(bool action, bool feedback)
{
    ESP_LOGI(_TAG, "action = %d, feedback = %d\n", action, feedback);
    _pulseOn.tick();
    _pulseOff.tick();
    if (action)
    {
        if (!feedback)
        {
            if (_failOn)
            {
                if (millis() - _lastFailOnCheck > _interval)
                {
                    _pulseOn.set();
                    _lastFailOnCheck = millis();
                }
            }
            else
            {
                if (!_pulseOn.isRunning())
                {
                    _failOnCnt++;
                    if (_failOnCnt > 10)
                    {
                        _failOn = true;
                        _lastFailOnCheck = millis();
                    }
                    ESP_LOGI(_TAG, "pulse relay on set\n");
                    _pulseOn.set();
                }
            }
        }
        else
        {
            _failOn = false;
            _failOnCnt = 0;
        }
    }
    else
    {
        ESP_LOGI(_TAG, "pulse off\n");
        if (feedback)
        {
            if (_failOff)
            {
                if (millis() - _lastFailOffCheck > _interval)
                {
                    _pulseOff.set();
                    _lastFailOffCheck = millis();
                }
            }
            else
            {
                if (!_pulseOff.isRunning())
                {
                    _failOffCnt++;
                    if (_failOffCnt > 10)
                    {
                        _failOff = true;
                        _lastFailOffCheck = millis();
                    }
                    ESP_LOGI(_TAG, "pulse relay off set\n");
                    _pulseOff.set();
                }
            }
        }
        else
        {
            _failOff = false;
            _failOffCnt = 0;
        }
    }
}

bool LatchHandle::isFailedOn()
{
    return _failOn;
}

bool LatchHandle::isFailedOff()
{
    return _failOff;
}

LatchHandle::~LatchHandle()
{
}