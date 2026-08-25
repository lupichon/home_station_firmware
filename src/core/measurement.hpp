/**
 * @file    measurement.hpp
 * @brief   Defines the sensor measurement data structure and its associated
 *          transmission status, along with a helper to reset a measurement.
 * @author  Lucas Pichon
 * @date    2026-07-23
 */

#pragma once

#include <math.h>

// ============================================================
// Measurement struct definition
// ============================================================

/**
 * @brief Holds a single set of sensor readings taken at a given timestamp.
 *        Fields default to NAN (float fields) or false (boolean fields) to
 *        represent "no reading yet" / "not applicable".
 */
struct Measurement
{
    uint32_t timestamp = 0;         // Epoch time at which the measurement was taken

    float temperature  = NAN;       // Temperature reading (°C)
    float humidity     = NAN;       // Relative humidity reading (%)
    float luminosity   = NAN;       // Luminosity reading (lux)
    float pressure     = NAN;       // Atmospheric pressure reading (hPa)
    uint16_t co2       = NAN;       // CO2 concentration reading (ppm)
    uint16_t gasRaw    = NAN;       // Raw gas sensor reading
    uint16_t vocIndex  = NAN;       // VOC (volatile organic compounds) index
    uint16_t noxIndex  = NAN;       // NOx (nitrogen oxides) index

    bool motion        = false;     // Whether motion was detected
    bool sound         = false;     // Whether sound was detected
    bool obstacle      = false;     // Whether an obstacle was detected
    bool vibration     = false;     // Whether vibration was detected
};

// ============================================================
// MeasurementStatus struct definition
// ============================================================

/**
 * @brief Reports whether a measurement was successfully read and/or
 *        successfully transmitted over each communication channel.
 */
struct MeasurementStatus
{
    bool sensorOK;      // Whether the sensor reading itself succeeded
    bool bluetoothOK;   // Whether the measurement was sent over Bluetooth successfully
    bool lorawanOK;     // Whether the measurement was sent over LoRaWAN successfully
};

// ============================================================
// Helpers
// ============================================================

/**
 * @brief Reset a Measurement to its default "empty" state (no timestamp,
 *        NAN readings, all flags false).
 * @param measurement Measurement instance to clear, in place.
 */
inline void clearMeasurement(Measurement& measurement)
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