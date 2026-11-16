/**
 * @file    sensor.hpp
 * @brief   BMP280 pressure sensor as seen by the application (sea-level correction).
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include "../sensor.hpp"
#include "driver.hpp"

// Pressure measurement modes for BMP280 sensor
#define PRESSURE_RAW 1          // Use raw atmospheric pressure without sea-level correction
#define PRESSURE_SEA_LEVEL 2    // Correct pressure to sea level using station altitude

#define PRESSURE_DEFAULT PRESSURE_SEA_LEVEL

#if PRESSURE_DEFAULT != PRESSURE_RAW && PRESSURE_DEFAULT != PRESSURE_SEA_LEVEL
#error "Unsupported PRESSURE_DEFAULT value"
#endif

// ============================================================
// BMP280Sensor class definition
// ============================================================

class BMP280Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        BMP280Driver driver;     // Low-level access to the chip
        float altitudeM = 0.0f;  // Station altitude in meters, used for sea-level pressure

    // ── Public interface ────────────────────────────────────────────────────
    public:

        BMP280Sensor();

        /**
         * @brief Initialize the underlying driver.
         * @return true if the sensor was initialized successfully, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Set the station altitude used to convert the pressure to sea level.
         * @param meters Altitude above sea level in meters.
         */
        void setAltitude(float meters);

        /**
         * @brief Read the atmospheric pressure into the measurement.
         * @param m Measurement that receives the pressure.
         * @return true if the measurement succeeded, false otherwise.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the pressure of a measurement for display.
         * @param m Measurement containing the pressure.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of BMP280Sensor methods
// ============================================================

inline BMP280Sensor::BMP280Sensor()
    : Sensor("BMP280", "hPa", BMP280_BIT)
{
}

inline bool BMP280Sensor::begin()
{
    initialized = driver.begin();

    return initialized;
}

inline void BMP280Sensor::setAltitude(float meters)
{
    altitudeM = meters;
}

inline bool BMP280Sensor::read(Measurement& m)
{
    float stationHpa;

    if (!initialized || !driver.readPressureHpa(stationHpa))
    {
        return false;
    }

    #if PRESSURE_DEFAULT == PRESSURE_RAW
    m.pressure = stationHpa;
    #endif

    #if PRESSURE_DEFAULT == PRESSURE_SEA_LEVEL
    m.pressure = stationHpa / powf(1.0f - altitudeM / 44330.0f, 5.255f);
    #endif

    return true;
}

inline const char* BMP280Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s : %.1f hPa", getName(), m.pressure);

    return buffer;
}
