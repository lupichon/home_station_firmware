#pragma once

#include <BH1750.h>
#include "../../core/sensor.hpp"

class BH1750Sensor : public Sensor
{
    private:
        BH1750 sensor;

    public:
        BH1750Sensor();
        bool begin() override;
        void read(Measurement &m) override;

    protected:
        float measuredValue(const Measurement &m) const override;
};