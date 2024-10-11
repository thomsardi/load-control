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
        void setup(uint8_t pin, int pulseOnDuration = 100, int pulseOffDuration = 100, bool activeLow = false); //setup object
        void set(); //set the pin
        void reset(); //reset the pin
        void changePulseOnDuration(int duration); //change on duration
        void changePulseOffDuration(int duration); //change off duration
        void changeActiveState(bool activeLow = false); //change mode
        void tick(); //main loop
        bool isRunning(); //check for running state
};

#endif