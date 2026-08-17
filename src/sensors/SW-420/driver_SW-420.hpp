#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

class SW420Sensor : public Sensor
{
    private:

        int pin;

        volatile bool vibrationDetected = false;

        static void IRAM_ATTR handleInterrupt(void* arg);

    public:

        SW420Sensor(int pin);

        bool begin() override;
        bool read(Measurement& m) override;

        const char* displayValue(const Measurement& m) const;
};

inline SW420Sensor::SW420Sensor(int pin)
    : Sensor("SW-420"), pin(pin)
{

}

inline bool SW420Sensor::begin()
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

inline void IRAM_ATTR SW420Sensor::handleInterrupt(void* arg)
{
    SW420Sensor* sensor =
        static_cast<SW420Sensor*>(arg);

    sensor->vibrationDetected = true;
}

inline bool SW420Sensor::read(Measurement& m)
{
    m.vibration = vibrationDetected;

    vibrationDetected = false;

    return true;
}

inline const char* SW420Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.vibration ? "Vibration detected"
                    : "No vibration"
    );

    return buffer;
}