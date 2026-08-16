#pragma once

#include <math.h>

struct Measurement
{
    uint32_t timestamp = 0;
    float temperature  = NAN;
    float humidity     = NAN;
    float luminosity   = NAN;
    float pressure     = NAN;
    uint16_t co2       = NAN;
    uint16_t gasRaw    = NAN;
    uint16_t vocIndex  = NAN; 
    uint16_t noxIndex  = NAN;
    bool  motion       = false;
    bool  sound        = false;
    bool  obstacle     = false;
    bool vibration     = false;
};

void clearMeasurement(Measurement& measurement)
{
    measurement.timestamp   = 0;
    measurement.temperature = NAN;
    measurement.humidity    = NAN;
    measurement.co2         = NAN;
    measurement.luminosity  = NAN;
    measurement.pressure    = NAN;
    measurement.gasRaw      = NAN;
    measurement.vocIndex    = NAN;
    measurement.noxIndex    = NAN;
    measurement.motion      = false;
    measurement.sound       = false;
    measurement.obstacle    = false;
    measurement.vibration   = false;
}