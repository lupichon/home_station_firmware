/**
 * @file    driver_BMP280.hpp
 * @brief   BMP280 barometric pressure sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "../sensor.hpp"

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
        Adafruit_BMP280 bmp;  // BMP280 sensor driver instance
        float altitudeM = 0.0f;  // Station altitude in meters, used for sea-level pressure

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for BMP280Sensor.
         */
        BMP280Sensor();

        /**
         * @brief Initialize the BMP280 sensor.
         * @return true if the sensor was initialized successfully,
         *         false otherwise.
         */
        bool begin() override;

        /**
         * @brief Set the station altitude used to convert the pressure to sea level.
         * @param meters Altitude above sea level in meters.
         */
        void setAltitude(float meters);

        /**
         * @brief Read the current atmospheric pressure.
         * @param m Measurement object that will receive the pressure value.
         * @return true if the measurement was successfully read,
         *         false if the sensor has not been initialized.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current pressure measurement for display.
         * @param m Measurement containing the pressure value to display.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of BMP280Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline BMP280Sensor::BMP280Sensor()
    : Sensor("BMP280")
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool BMP280Sensor::begin()
{
    // Initialize the BMP280 using I2C address 0x76
    initialized = bmp.begin(0x76);

    if (initialized)
    {
        bmp.setSampling(
            Adafruit_BMP280::MODE_FORCED,
            Adafruit_BMP280::SAMPLING_X16,
            Adafruit_BMP280::SAMPLING_X16,
            Adafruit_BMP280::FILTER_OFF,
            Adafruit_BMP280::STANDBY_MS_1
        );
    }

    return initialized;
}

// ─────────────────────────────────────────────────────────────────────────────
// Setter
// ─────────────────────────────────────────────────────────────────────────────

inline void BMP280Sensor::setAltitude(float meters)
{
    altitudeM = meters;
}

// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool BMP280Sensor::read(Measurement& m)
{
    // Do not attempt to read an uninitialized sensor
    if (!initialized)
    {
        return false;
    }

    if(!bmp.takeForcedMeasurement())
    {
        return false;
    }

    // Convert pressure from Pa to hPa, then reduce it to sea level
    float stationHpa = bmp.readPressure() / 100.0f;

    #if PRESSURE_DEFAULT == PRESSURE_RAW
    m.pressure = stationHpa;
    #endif

    #if PRESSURE_DEFAULT == PRESSURE_SEA_LEVEL
    m.pressure = stationHpa / powf(1.0f - altitudeM / 44330.0f, 5.255f);
    #endif

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* BMP280Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name, pressure value and unit
    snprintf(
        buffer,
        sizeof(buffer),
        "%s : %.1f hPa",
        getName(),
        m.pressure
    );

    return buffer;
}