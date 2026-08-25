#pragma once

#include <stdio.h>
#include "../core/measurement.hpp"

class Sensor
{
    protected:
        bool initialized = false;

    private:
        char sensorName[32];
        char unit[16];
        static inline int sensorCount;

    public:
        Sensor();
        Sensor(const char* name);
        Sensor(const char* name, const char* unit);

        virtual ~Sensor();

        const char* getName() const;
        const char* getUnit() const;
        bool isInitialized() const;
        static int getSensorCount();

        virtual bool begin() = 0;
        virtual bool read(Measurement& m) = 0;

        virtual const char* displayValue(const Measurement& m) const = 0;
};

inline Sensor::Sensor() : Sensor("", "")
{
    snprintf(sensorName, sizeof(sensorName), "Sensor_%d", sensorCount-1);
}

inline Sensor::Sensor(const char* name) : Sensor(name, "")
{

}

inline Sensor::Sensor(const char* name, const char* unit)
{
    snprintf(sensorName, sizeof(sensorName), "%s", name);
    snprintf(this->unit, sizeof(this->unit), "%s", unit);

    Sensor::sensorCount++;
}

inline const char* Sensor::getName() const
{
    return sensorName;
}

inline const char* Sensor::getUnit() const
{
    return unit;
}

inline int Sensor::getSensorCount()
{
    return sensorCount;
}

inline bool Sensor::isInitialized() const
{
    return initialized;
}

inline Sensor::~Sensor()
{
    sensorCount--;
}