#pragma once

#include <Arduino.h>
#include "../sensor.hpp"


class MQ2Sensor : public Sensor
{
    private:
        uint8_t pin;

    public:

        MQ2Sensor(uint8_t pin);
        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const;
};



inline MQ2Sensor::MQ2Sensor(uint8_t pin)
    : Sensor("MQ-2"),
      pin(pin)
{

}



inline bool MQ2Sensor::begin()
{
    pinMode(pin, INPUT);
    analogReadResolution(12);

    initialized = true;

    return true;
}



inline bool MQ2Sensor::read(Measurement& m)
{
    uint16_t gasValue = analogRead(pin);
    m.gasRaw = gasValue;
    return true;
}



inline const char* MQ2Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %d",
        getName(),
        m.gasRaw
    );


    return buffer;
}