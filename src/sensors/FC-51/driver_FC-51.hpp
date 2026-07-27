#pragma once

#include <Arduino.h>
#include "../sensor.hpp"

//TODO : Régler le potentiomètre pour régler la distance de détection de l'obstacle

class FC51Sensor : public Sensor
{
    private:
        int pin;

    public:
        FC51Sensor(int pin);

        bool begin() override;
        void read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const override;
};


inline FC51Sensor::FC51Sensor(int pin)
    : Sensor("FC-51"),
      pin(pin)
{
}


inline bool FC51Sensor::begin()
{
    pinMode(pin, INPUT);

    initialized = true;

    return initialized;
}


inline void FC51Sensor::read(Measurement& m)
{
    m.obstacle = (digitalRead(pin) == LOW);
}

inline const char* FC51Sensor::displayValue(const Measurement& m) const
{
    return m.obstacle ? "Obstacle detected" : "No obstacle";
}