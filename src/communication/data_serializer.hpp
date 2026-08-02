#pragma once

#include <stdint.h>
#include <string.h>
#include "../core/measurement.hpp"
#include "../core/gas_state.hpp"

constexpr size_t NB_FLOATS    = 3;
constexpr size_t NB_UINT16    = 1;
constexpr size_t NB_FLAG_BYTE = 1;
constexpr size_t BUFFER_SIZE = (sizeof(float) * NB_FLOATS)
                              + (sizeof(uint16_t) * NB_UINT16)
                              + (sizeof(uint8_t) * NB_FLAG_BYTE);
                              
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

    memcpy(buffer + offset, &measurement.luminosity, sizeof(float));
    offset += sizeof(float);
    
    memcpy(buffer + offset, &measurement.co2, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    uint8_t flags = 0;
    flags |= (measurement.motion   ? 1 : 0) << 0;
    flags |= (measurement.sound    ? 1 : 0) << 1;
    flags |= (measurement.obstacle ? 1 : 0) << 2;
    flags |= (measurement.vibration ? 1 : 0) << 3;
    flags |= (static_cast<uint8_t>(gasStateFromRaw(measurement.gasRaw)) & 0x03) << 4;

    buffer[offset] = flags;
    offset += sizeof(uint8_t);

    return offset;
}
