#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

// TODO : A TESTER QUAND LE CAPTEUR SERA SOUDé (REGLER LE SEUIL AUSSI)

class MAX9814Sensor : public Sensor
{
    private:
        static constexpr uint32_t TIMER_PERIOD_US = 100'000;
        static constexpr int SOUND_THRESHOLD = 2000;

        int pin;

        hw_timer_t* timer = nullptr;

        volatile bool soundDetected = false;

        static void onTimer(void* arg);

    public:
        MAX9814Sensor(int pin);

        bool begin() override;
        void read(Measurement& m) override;
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

    timer = timerBegin(1000000);

    if (timer == nullptr)
    {
        initialized = false;
        return false;
    }

    timerAttachInterruptArg(timer, onTimer, this);
    timerAlarm(timer, TIMER_PERIOD_US, true, 0);

    initialized = true;

    return true;
}

inline void MAX9814Sensor::onTimer(void* arg)
{
    MAX9814Sensor* sensor = static_cast<MAX9814Sensor*>(arg);

    int value = analogRead(sensor->pin);

    if (value > SOUND_THRESHOLD)
    {
        sensor->soundDetected = true;
    }
}

inline void MAX9814Sensor::read(Measurement& m)
{
    m.sound = soundDetected;
    soundDetected = false;
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