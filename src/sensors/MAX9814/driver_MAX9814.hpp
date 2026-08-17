#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

class MAX9814Sensor : public Sensor
{
    private:
        int pin;
        bool soundDetected = false;
        int signalMax = 0; 
        int signalMin = 4095;
        unsigned long windowStart = 0;
        static constexpr unsigned long WINDOW_MS = 60; 
        static constexpr int THRESHOLD = 500;

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
    int sample = analogRead(pin);

    if (sample > signalMax) signalMax = sample;
    if (sample < signalMin) signalMin = sample;

    if (millis() - windowStart >= WINDOW_MS)
    {
        int amplitude = signalMax - signalMin;
        soundDetected = (amplitude > THRESHOLD);

        signalMax = 0;
        signalMin = 4095;
        windowStart = millis();
    }
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