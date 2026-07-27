#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

class HCSR501Sensor : public Sensor
{
    private:
        int pin;
        volatile bool motionDetected = false; // Indicates if motion has been detected since the last read

        static void IRAM_ATTR handleInterrupt(void* arg);

    public:
        HCSR501Sensor(int pin);

        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const;
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

inline bool HCSR501Sensor::read(Measurement& m)
{
    // Update the motion value in the Measurement struct
    m.motion = motionDetected;

    // Reset the motionDetected flag after reading
    motionDetected = false;
    return true;
}

inline void IRAM_ATTR HCSR501Sensor::handleInterrupt(void* arg)
{
    HCSR501Sensor* sensor = static_cast<HCSR501Sensor*>(arg);
    sensor->motionDetected = true;
}

inline const char* HCSR501Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.motion ? "Motion detected" : "No motion"
    );

    return buffer;
}
