#pragma once

#include <stdint.h>
#include <string.h>
#include "../core/measurement.hpp"

constexpr size_t NB_FLOATS = 4;
constexpr size_t NB_BOOL   = 3;
constexpr size_t BUFFER_SIZE = (sizeof(float) * NB_FLOATS + sizeof(uint8_t) * NB_BOOL);

size_t serialize(const Measurement& measurement, uint8_t* buffer, size_t bufferSize)
{
    if (buffer == nullptr || bufferSize < BUFFER_SIZE)
    {
        return 0;
    }

    size_t offset = 0;

    memcpy(buffer + offset, &measurement.temperature, sizeof(float));
    offset += sizeof(float);

    memcpy(buffer + offset, &measurement.humidity, sizeof(float));
    offset += sizeof(float);

    memcpy(buffer + offset, &measurement.co2, sizeof(float));
    offset += sizeof(float);

    memcpy(buffer + offset, &measurement.luminosity, sizeof(float));
    offset += sizeof(float);

    buffer[offset] = measurement.motion ? 1 : 0;
    offset += sizeof(uint8_t);

    buffer[offset] = measurement.sound ? 1 : 0;
    offset += sizeof(uint8_t);

    buffer[offset] = measurement.obstacle ? 1 : 0;
    offset += sizeof(uint8_t);

    return offset;
}