#ifndef PULSE_OUTPUT_H
#define PULSE_OUTPUT_H

#include <Arduino.h>

class PulseOutput {
    private :
        const char* _TAG = "pulse-output";
        uint8_t _pin;
        int _pulseOnDuration = 100;
        int _pulseOffDuration = 100;
        unsigned long _lastPulseOnCheck;
        unsigned long _lastPulseOffCheck;
        bool _activeLow = false;
        bool _isSet = false;
    public :
        PulseOutput();
        void setup(uint8_t pin, int pulseOnDuration = 100, int pulseOffDuration = 100, bool activeLow = false);
        void set();
        void reset();
        void changePulseOnDuration(int duration);
        void changePulseOffDuration(int duration);
        void changeActiveState(bool activeLow = false);
        void tick();
        bool isRunning();
};

#endif