/**
 * @file    data_serializer.hpp
 * @brief   Serializes sensor measurements into a compact binary format.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <stdint.h>
#include <string.h>

#include "../core/measurement.hpp"
#include "../core/gas_state.hpp"

// ============================================================
// Buffer layout constants
// ============================================================

constexpr size_t NB_FLOATS    = 4; // Number of float fields    (temperature, humidity, luminosity, pressure)
constexpr size_t NB_UINT16    = 3; // Number of uint16_t fields (co2, vocIndex, noxIndex)
constexpr size_t NB_FLAG_BYTE = 1; // Number of flag bytes      (motion, sound, obstacle, vibration, gas state)

constexpr size_t BUFFER_SIZE = (sizeof(float)    * NB_FLOATS)
                             + (sizeof(uint16_t)  * NB_UINT16)
                             + (sizeof(uint8_t)   * NB_FLAG_BYTE);

// ============================================================
// Serialization
// ============================================================

/**
 * @brief Serialize a Measurement into a compact binary buffer.
 * @param measurement  Measurement to serialize.
 * @param buffer       Destination buffer.
 * @param bufferSize   Size of the destination buffer in bytes.
 * @return Number of bytes written, or 0 if the buffer is null or too small.
 */
inline size_t serialize(const Measurement& measurement, uint8_t* buffer, size_t bufferSize)
{
    if (buffer == nullptr || bufferSize < BUFFER_SIZE) return 0;

    size_t offset = 0;

    // Floats
    memcpy(buffer + offset, &measurement.temperature, sizeof(float)); offset += sizeof(float);
    memcpy(buffer + offset, &measurement.humidity,    sizeof(float)); offset += sizeof(float);
    memcpy(buffer + offset, &measurement.luminosity,  sizeof(float)); offset += sizeof(float);
    memcpy(buffer + offset, &measurement.pressure,    sizeof(float)); offset += sizeof(float);

    // uint16_t
    memcpy(buffer + offset, &measurement.co2,      sizeof(uint16_t)); offset += sizeof(uint16_t);
    memcpy(buffer + offset, &measurement.vocIndex, sizeof(uint16_t)); offset += sizeof(uint16_t);
    memcpy(buffer + offset, &measurement.noxIndex, sizeof(uint16_t)); offset += sizeof(uint16_t);

    // Flag byte: bits 0-3 = binary sensors, bits 4-5 = gas state
    uint8_t flags = 0;
    flags |= (measurement.motion    ? 1 : 0) << 0;
    flags |= (measurement.sound     ? 1 : 0) << 1;
    flags |= (measurement.obstacle  ? 1 : 0) << 2;
    flags |= (measurement.vibration ? 1 : 0) << 3;
    flags |= (static_cast<uint8_t>(gasStateFromRaw(measurement.gasRaw)) & 0x03) << 4;

    buffer[offset] = flags; offset += sizeof(uint8_t);

    return offset;
}