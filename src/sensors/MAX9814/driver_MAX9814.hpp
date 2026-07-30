#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

// TODO : A TESTER QUAND LE CAPTEUR SERA SOUDé (REGLER LE SEUIL AUSSI)

class MAX9814Sensor : public Sensor
{
    private:
        int pin;
        bool soundDetected = false;

    public:
        MAX9814Sensor(int pin);
        void update();

        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const override;
};

inline MAX9814Sensor::MAX9814Sensor(int pin)
    : Sensor("MAX9814"),
      pin(pin)
{
}

inline bool MAX9814Sensor::begin()
{
    pinMode(pin, INPUT);
    analogReadResolution(12);

    initialized = true;

    return true;
}

inline void MAX9814Sensor::update()
{
    int soundLevel = analogRead(pin);
    soundDetected = (soundLevel > 1000); // Adjust the threshold as needed
}

inline bool MAX9814Sensor::read(Measurement& m)
{
    m.sound = soundDetected;
    soundDetected = false;
    return true;
}

inline const char* MAX9814Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.sound ? "Sound detected" : "No sound"
    );

    return buffer;
}