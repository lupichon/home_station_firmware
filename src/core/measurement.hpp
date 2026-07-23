#pragma once

#include <math.h>

struct Measurement
{
    float temperature = NAN;
    float humidity    = NAN;
    float pressure    = NAN;
    float co2         = NAN;
    float luminosity  = NAN;
    bool  motion      = false;
};