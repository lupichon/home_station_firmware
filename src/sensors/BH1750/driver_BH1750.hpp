#pragma once

#include <BH1750.h>
#include "../sensor.hpp"

class BH1750Sensor : public Sensor
{
    private:
        BH1750 sensor;

    public:
        BH1750Sensor();

        bool begin() override;
        void read(Measurement& m) override;

    protected:
        float measuredValue(const Measurement& m) const override;
};


inline BH1750Sensor::BH1750Sensor()
    : Sensor("BH1750", "lux")
{
}

inline bool BH1750Sensor::begin()
{
    initialized = sensor.begin();
    return initialized;
}

inline void BH1750Sensor::read(Measurement& m)
{
    m.luminosity = sensor.readLightLevel();
}

inline float BH1750Sensor::measuredValue(const Measurement& m) const
{
    return m.luminosity;
}