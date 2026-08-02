#pragma once

#include <math.h>

struct Measurement
{
    float temperature = NAN;
    float humidity    = NAN;
    uint16_t co2      = NAN;
    float luminosity  = NAN;
    uint16_t gasRaw   = NAN;
    bool  motion      = false;
    bool  sound       = false;
    bool  obstacle    = false;
    bool vibration    = false;
};

void clearMeasurement(Measurement& measurement)
{
    measurement.temperature = NAN;
    measurement.humidity    = NAN;
    measurement.co2         = NAN;
    measurement.luminosity  = NAN;
    measurement.gasRaw      = NAN;
    measurement.motion      = false;
    measurement.sound       = false;
    measurement.obstacle    = false;
    measurement.vibration   = false;
}