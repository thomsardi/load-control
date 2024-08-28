#include "latchhandle.h"

LatchHandle::LatchHandle(/* args */)
{
}

/**
 * Setup class
 * 
 * @param[in]   id  id of class
 * @param[in]   retryInterval interval when failed trigger relay
 * @param[in]   maxRetry maximum number of retry
 * @param[in]   pulseOn pointer into PulseOutput data type
 * @param[in]   pulseOff pointer into PulseOutput data type
 */
void LatchHandle::setup(uint8_t id, int retryInterval, int maxRetry, PulseOutput *pulseOn, PulseOutput *pulseOff)
{
    _id = id;
    _interval = retryInterval;
    _maxRetry = maxRetry;
    _pulseOn = pulseOn;
    _pulseOff = pulseOff;
}

/**
 * On signal callback
 * 
 * @param[in]   cb callback
 */
void LatchHandle::onSignal(Callback cb)
{
    _onSignalCb = cb;
}

/**
 * Set manual
 */
void LatchHandle::setManual()
{
    _isManual = true;
}

/**
 * Set auto
 */
void LatchHandle::setAuto()
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
void LatchHandle::stop()
{
    _isStop = true;
}

/**
 * Restart
 */
void LatchHandle::restart()
{
    _failOnCnt = 0;
    _failOn = false;
    _failOffCnt = 0;
    _failOff = false;
}

/**
 * Reset pulse on state
 * 
 * @brief   use this method to reset the flag, so the class routine can continue
 */
void LatchHandle::resetPulseOn()
{
    _pulseOnState = false;
}

/**
 * Reset pulse off state
 * 
 * @brief   use this method to reset the flag, so the class routine can continue
 */
void LatchHandle::resetPulseOff()
{
    _pulseOffState = false;
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
void LatchHandle::handle(bool action, bool feedback)
{
    if(_isStop)
    {
        return;
    }

    if(!_isManual)
    {
        if (action) //if action is HIGH (ON)
        {
            _pulseOffState = false;
            if (!feedback) //if relay feedback is off
            {
                if (_failOn) //if in failstate
                {
                    if (millis() - _lastFailOnCheck > _interval)
                    {
                        _lastFailOnCheck = millis();
                        if(_pulseOnState) //if the state is not resetted, immediately return
                        {
                            return;
                        }
                        _pulseOnState = true;
                        Latch::latch_sync_signal_t signal; //build data to pass into callback
                        signal.id = _id;
                        signal.pulseOn = _pulseOn;
                        if (_onSignalCb) //if on signal callback is exist (not null pointer)
                        {
                            ESP_LOGI(_TAG, "relay on");
                            _onSignalCb(signal); //call the on signal callback
                        }
                    }
                }
                else
                {
                    if (_pulseOnState) //if the state is not resetted, immediately return
                    {
                        return;
                    }
                    _pulseOnState = true;
                    Latch::latch_sync_signal_t signal; //build data to pass into callback
                    signal.id = _id;
                    signal.pulseOn = _pulseOn;
                    if (_onSignalCb) //if on signal callback is exist (not null pointer)
                    {
                        ESP_LOGI(_TAG, "relay on");
                        _onSignalCb(signal); //call the on signal callback
                    }
                    _failOnCnt++; //because action high and feedback still low, count it as fail
                    if (_failOnCnt > _maxRetry) //if too many failed trigger, enter fail state
                    {
                        _failOn = true;
                        _lastFailOnCheck = millis();
                    }
                }
            }
            else
            {
                _failOn = false;
                _failOnCnt = 0;
                _pulseOnState = false;
            }
        }
        else //if action is LOW
        {
            _pulseOnState = false;
            if (feedback) //if feedback is HIGH
            {
                if (_failOff) //if in fail state
                {
                    if (millis() - _lastFailOffCheck > _interval)
                    {
                        _lastFailOffCheck = millis();
                        if (_pulseOffState) //if the state is not resetted, immediately return
                        {
                            return;
                        }
                        _pulseOffState = true;
                        Latch::latch_sync_signal_t signal; //build data to pass into callback
                        signal.id = _id;
                        signal.pulseOff = _pulseOff;
                        _pulseOffState = false;
                        if (_onSignalCb) //if on signal callback is exist (not null pointer)
                        {
                            ESP_LOGI(_TAG, "relay off");
                            _onSignalCb(signal); //call the on signal callback
                        }
                    }
                }
                else
                {
                    if (_pulseOffState) //if the state is not resetted, immediately return
                    {
                        return;
                    }
                    _pulseOffState = true;
                    Latch::latch_sync_signal_t signal; //build data to pass into callback
                    signal.id = _id;
                    signal.pulseOff = _pulseOff;
                    if (_onSignalCb) //if on signal callback is exist (not null pointer)
                    {
                        ESP_LOGI(_TAG, "relay off");
                        _onSignalCb(signal); //call the on signal callback
                    }
                    _failOffCnt++;
                    if (_failOffCnt > _maxRetry)
                    {
                        _failOff = true;
                        _lastFailOffCheck = millis();
                    }
                    ESP_LOGI(_TAG, "pulse relay off set\n");
                }
            }
            else
            {
                _failOffCnt = 0;
                _failOff = false;
                _pulseOffState = false;
            }
        }
    }    
}

/**
 * Get id of class
 * 
 * @return  id of the class
 */
uint8_t LatchHandle::getId()
{
    return _id;
}

/**
 * Get relay ON failed status
 * 
 * @return  state of relay ON failed
 */
bool LatchHandle::isFailedOn()
{
    return _failOn;
}

/**
 * Get relay OFF failed status
 * 
 * @return  state of relay OFF failed
 */
bool LatchHandle::isFailedOff()
{
    return _failOff;
}

LatchHandle::~LatchHandle()
{
}