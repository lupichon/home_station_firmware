#pragma once

#include <stdio.h>
#include "measurement.hpp"

class Sensor
{
    private:
        const char *sensorName; 
        const char *unit; 

    public:
        Sensor(const char *name, const char *unit) : sensorName(name), unit(unit) {}

        const char *getName() const { return sensorName; }
        const char *getUnit() const { return unit; }

        virtual bool begin() = 0;
        virtual void read(Measurement& m) = 0;

        virtual const char *displayValue(const Measurement& m)
        {
            static char buffer[32];
            snprintf(buffer, sizeof(buffer), "%s: %.2f %s", getName(), measuredValue(m), getUnit());
            return buffer;
        }

    protected:
        virtual float measuredValue(const Measurement& m) const = 0;

    public:
        virtual ~Sensor() {}
};