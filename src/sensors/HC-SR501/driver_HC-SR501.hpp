#pragma once

#include <Arduino.h>
#include "../../core/sensor.hpp"

class HCSR501Sensor : public Sensor
{
    private:
        int pin;
        volatile bool motionDetected = false; // Indicates if motion has been detected since the last read

        static void IRAM_ATTR handleInterrupt(void* arg);

    public:
        HCSR501Sensor(int pin);

        bool begin() override;
        void read(Measurement& m) override;

    protected:
        float measuredValue(const Measurement& m) const override;
};


inline HCSR501Sensor::HCSR501Sensor(int pin)
    : Sensor("HC-SR501"), pin(pin)
{

}

inline bool HCSR501Sensor::begin()
{
    pinMode(pin, INPUT);

    int interruptPin = digitalPinToInterrupt(pin);

    if (interruptPin == NOT_AN_INTERRUPT)
    {
        initialized = false;
    }
    else
    {
        attachInterruptArg(
            interruptPin,
            handleInterrupt,
            this,
            RISING
        );
        initialized = true;
    }

    return initialized;
}

inline void HCSR501Sensor::read(Measurement& m)
{
    // Update the motion value in the Measurement struct
    m.motion = motionDetected;

    // Reset the motionDetected flag after reading
    motionDetected = false;
}

inline float HCSR501Sensor::measuredValue(const Measurement& m) const
{
    return m.motion ? 1.0f : 0.0f; 
}

inline void IRAM_ATTR HCSR501Sensor::handleInterrupt(void* arg)
{
    HCSR501Sensor* sensor = static_cast<HCSR501Sensor*>(arg);
    sensor->motionDetected = true;
}
