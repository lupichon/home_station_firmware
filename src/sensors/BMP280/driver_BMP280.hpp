#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "../sensor.hpp"

//TODO: A TESTER

class BMP280Sensor : public Sensor
{
private:
    Adafruit_BMP280 bmp;

public:
    BMP280Sensor();

    bool begin() override;
    bool read(Measurement& m) override;
    const char* displayValue(const Measurement& m) const;
};

inline BMP280Sensor::BMP280Sensor()
    : Sensor("BMP280")
{
}

inline bool BMP280Sensor::begin()
{

    initialized = bmp.begin(0x76);
    if(initialized)
    {
        bmp.setSampling(
            Adafruit_BMP280::MODE_NORMAL,
            Adafruit_BMP280::SAMPLING_X16,
            Adafruit_BMP280::SAMPLING_X16,
            Adafruit_BMP280::FILTER_X16,
            Adafruit_BMP280::STANDBY_MS_1
        );
    }

    return initialized;
}

inline bool BMP280Sensor::read(Measurement& m)
{
    if (!initialized)
    {
        return false;
    }

    m.pressure = bmp.readPressure() / 100.0f; // hPa
    return true;
}

inline const char* BMP280Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s : %.1f hPa",
        getName(),
        m.pressure
    );

    return buffer;
}