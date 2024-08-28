#include "latchhandle.h"

LatchHandleAsync::LatchHandleAsync(/* args */)
{
}

/**
 * Write config parameter into private member of class
 * 
 * @param[in]   config  struct type of latch_async_config_t
 */
void LatchHandleAsync::setup(const Latch::latch_async_config_t &config)
{
    _pinOn = config.pinOn;
    _pinOff = config.pinOff;
    _onDuration = config.onDuration;
    _offDuration = config.offDuration;
    _activeLow = config.activeLow;
    _retryInterval = config.retryInterval;
    _maxRetry = config.maxRetry;
    _pulseOn.setup(_pinOn, _onDuration, _offDuration, _activeLow);
    _pulseOff.setup(_pinOff, _onDuration, _offDuration, _activeLow);
}

/**
 * Set manual
 */
void LatchHandleAsync::setManual()
{
    if (!_isManual)
    {
        _pulseOn.reset();
        _pulseOff.reset();
    }
    _isManual = true;
}

/**
 * Set auto
 */
void LatchHandleAsync::setAuto()
{
    if (_isManual)
    {
        restart();
    }
    _isManual = false;
}

/**
 * Stop
 */
void LatchHandleAsync::stop()
{
    if (!_isStop)
    {
        _pulseOn.reset();
        _pulseOff.reset();
    }
    _isStop = true;
}

/**
 * Restart
 */
void LatchHandleAsync::restart()
{
    _failOnCnt = 0;
    _failOn = false;
    _failOffCnt = 0;
    _failOff = false;
}

/**
 * Set the pulse on
 */
void LatchHandleAsync::set()
{
    _isSet = true;
}

/**
 * Set pulse off
 */
void LatchHandleAsync::reset()
{
    _isReset = true;
}

/**
 * Main handle
 * 
 * @brief   call this function periodically to update the class value, will determine the action based on feedback. e.g when action HIGH, and the feedback LOW, it means
 *          that the relay still in not in the right state, so it will send pulse into relay
 * 
 * @param[in]   action  action status, it is either HIGH or LOW
 * @param[in]   feedback    feedback status, HIGH or LOW
 */
void LatchHandleAsync::handle(bool action, bool feedback)
{
    // ESP_LOGI(_TAG, "action = %d, feedback = %d\n", action, feedback);

    _pulseOn.tick();
    _pulseOff.tick();
    
    if(_isStop)
    {
        return;
    }

    if(!_isManual)
    {
        if (action) //if action HIGH
        {
            if (!feedback) //if feedback is LOW
            {
                if (_failOn) //if in failstate
                {
                    if (millis() - _lastFailOnCheck > _retryInterval)
                    {
                        _pulseOn.set();
                        _lastFailOnCheck = millis();
                    }
                }
                else
                {
                    if (!_pulseOn.isRunning()) //if pulse is in idle state, set the pulse
                    {
                        _failOnCnt++;
                        if (_failOnCnt > _maxRetry) //check for fail trigger
                        {
                            _failOn = true;
                            _lastFailOnCheck = millis();
                        }
                        ESP_LOGI(_TAG, "pulse relay on set\n");
                        _pulseOn.set(); //set the pulse
                    }
                }
            }
            else //if feedback is HIGH, it means that the relay is in right state
            {
                _failOn = false;
                _failOnCnt = 0;
            }
        }
        else //if action is LOW
        {
            if (feedback) //if feedback is HIGH
            {
                if (_failOff) //if in fail state
                {
                    if (millis() - _lastFailOffCheck > _retryInterval)
                    {
                        _pulseOff.set();
                        _lastFailOffCheck = millis();
                    }
                }
                else
                {
                    if (!_pulseOff.isRunning()) //if pulse is in idle state, set the pulse
                    {
                        _failOffCnt++;
                        if (_failOffCnt > _maxRetry) //check for fail trigger
                        {
                            _failOff = true;
                            _lastFailOffCheck = millis();
                        }
                        ESP_LOGI(_TAG, "pulse relay off set\n");
                        _pulseOff.set(); //set the pulse
                    }
                }
            }
            else //if feedback is LOW, it means that the relay is in right state
            {
                _failOff = false;
                _failOffCnt = 0;
            }
        }
    }
    else //if in manual
    {
        if (_isSet) //check for set flag
        {
            _pulseOn.set();
            _isSet = false;
        }

        if(_isReset) //check for reset flag
        {
            _pulseOff.set();
            _isReset = false;
        }
    }
    
}

/**
 * Get relay ON failed status
 * 
 * @return  state of relay ON failed
 */
bool LatchHandleAsync::isFailedOn()
{
    return _failOn;
}

/**
 * Get relay OFF failed status
 * 
 * @return  state of relay OFF failed
 */
bool LatchHandleAsync::isFailedOff()
{
    return _failOff;
}

LatchHandleAsync::~LatchHandleAsync()
{
}