#ifndef LATCH_HANDLE_H
#define LATCH_HANDLE_H

#include <Arduino.h>
#include "pulseoutput.h"

namespace Latch {
    /**
     * config struct for async latch
     */
    struct latch_async_config_t {
        int pinOn = -1; //pin on
        int pinOff = -1; //pin off
        int onDuration = 100; //ON duration 100ms
        int offDuration = 100; //OFF duration 100ms
        bool activeLow = false; //mode
        int retryInterval = 2000; //retry interval when failed to trigger the relay
        int maxRetry = 5; //maximum retry when no feedback received
    };

    /**
     * config struct for sync latch
     */
    struct latch_sync_config_t
    {
        uint8_t id = 0; //id of the class
        int retryInterval = 2000; //retry interval when failed to trigger the relay
        int maxRetry = 5; //maximum retry when no feedback received
        PulseOutput *pulseOn = NULL; //pointer to PulseOutput data type
        PulseOutput *pulseOff = NULL; //pointer to PulseOutput data type
    };
    
    /**
     * signal data struct
     */
    struct latch_sync_signal_t {
        uint8_t id = 0; //id of the class
        PulseOutput *pulseOn = NULL; //pointer to PulseOutput data type
        PulseOutput *pulseOff = NULL; //pointer to PulseOutput data type
    };
};

using Callback = std::function<void(Latch::latch_sync_signal_t signal)>; //function declaration for callback

/**
 * Sync latch handle, need to be controlled by reset some flag to re-trigger output
 */
class LatchHandle
{
private:
    /* data */
    const char* _TAG = "latch-handle";
    PulseOutput *_pulseOn;
    PulseOutput *_pulseOff;
    uint8_t _id = 1;
    Callback _onSignalCb;
    int _interval;
    int _failOnCnt;
    int _failOffCnt;
    int _maxRetry;
    bool _failOn;
    bool _failOff;
    bool _isStop;
    bool _isManual;
    bool _pulseOnState = false;
    bool _pulseOffState = false;
    unsigned long _lastFailOnCheck;
    unsigned long _lastFailOffCheck;
public:
    LatchHandle();
    uint8_t getId(); //get id of class
    void setup(const Latch::latch_sync_config_t &config); //setup class
    void setManual(); //set to manual
    void setAuto(); //set to auto
    void stop(); //stop routine
    void restart(); //restart
    void handle(bool action, bool feedback); //main handle
    void onSignal(Callback cb); //register callback when receive signal
    void resetPulseOn(); //reset pulse on flag
    void resetPulseOff(); //reset pulse off flag
    bool isFailedOn(); //get failed relay on
    bool isFailedOff(); //get failed relay off
    ~LatchHandle();
};

/**
 * Async latch handle class, can be called separately and handle the pulse output itself
 */
class LatchHandleAsync
{
private:
    /* data */
    const char* _TAG = "latch-handle-async";
    PulseOutput _pulseOn;
    PulseOutput _pulseOff;
    int _pinOn;
    int _pinOff;
    int _onDuration;
    int _offDuration;
    int _retryInterval;
    int _failOnCnt;
    int _failOffCnt;
    int _maxRetry;
    bool _activeLow;
    bool _failOn;
    bool _failOff;
    bool _isStop;
    bool _isManual;
    bool _isSet;
    bool _isReset;
    unsigned long _lastFailOnCheck;
    unsigned long _lastFailOffCheck;
public:
    LatchHandleAsync(/* args */);
    void setup(const Latch::latch_async_config_t &config); //setup class with config struct
    void setManual(); //set to manual
    void setAuto(); //set to auto
    void set(); //set pulse on
    void reset(); //set pulse off
    void stop(); //stop class
    void restart(); //restart
    void handle(bool action, bool feedback); //main handle
    bool isFailedOn(); //get failed relay on
    bool isFailedOff(); //get failed relay off
    ~LatchHandleAsync();
};


#endif