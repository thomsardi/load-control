#ifndef LATCH_HANDLE_H
#define LATCH_HANDLE_H

#include <Arduino.h>
#include "pulseoutput.h"

class LatchHandle
{
private:
    /* data */
    const char* _TAG = "latch-handle";
    PulseOutput _pulseOn;
    PulseOutput _pulseOff;
    int _pinOn;
    int _pinOff;
    int _feedbackPin;
    int _onDuration;
    int _offDuration;
    int _interval;
    int _failOnCnt;
    int _failOffCnt;
    bool _activeLow;
    bool _failOn;
    bool _failOff;
    unsigned long _lastFailOnCheck;
    unsigned long _lastFailOffCheck;
public:
    LatchHandle(/* args */);
    void setup(int pinOn, int pinOff, int onDuration = 50, int offDuration = 50, bool activeLow = false, int interval = 2000);
    void setDuration(int onDuration, int offDuration);
    void setRetryonFail(int interval);
    void setActiveState(bool activeLow);
    void handle(bool action, bool feedback);
    bool isFailedOn();
    bool isFailedOff();
    ~LatchHandle();
};

#endif