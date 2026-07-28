#pragma once

#include <math.h>

struct Measurement
{
    float temperature = NAN;
    float humidity    = NAN;
    float co2         = NAN;
    float luminosity  = NAN;
    bool  motion      = false;
    bool  sound       = false;
    bool  obstacle    = false;
};

void clearMeasurement(Measurement& measurement)
{
    measurement.temperature = NAN;
    measurement.humidity    = NAN;
    measurement.co2         = NAN;
    measurement.luminosity  = NAN;
    measurement.motion      = false;
    measurement.sound       = false;
    measurement.obstacle    = false;
}