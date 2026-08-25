/**
 * @file    driver_BH1750.hpp
 * @brief   BH1750 ambient light sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-16
 */

#pragma once

#include <BH1750.h>
#include "../sensor.hpp"


// ============================================================
// BH1750Sensor class definition
// ============================================================

/**
 * @brief Sensor implementation for measuring ambient light intensity.
 *
 * The BH1750Sensor class interfaces with a BH1750 digital light sensor
 * through the I2C bus and provides the measured luminosity in lux.
 *
 * The sensor operates in continuous high-resolution mode.
 */
class BH1750Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        BH1750 sensor;  // BH1750 sensor driver instance

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for BH1750Sensor.
         */
        BH1750Sensor();

        /**
         * @brief Initialize the BH1750 sensor.
         * @return true if the sensor was initialized successfully,
         *         false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read the current ambient light level.
         * @param m Measurement object that will receive the luminosity value.
         * @return true if the measurement was successfully read,
         *         false if the sensor returned an invalid value.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current luminosity measurement for display.
         * @param m Measurement containing the luminosity value to display.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of BH1750Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline BH1750Sensor::BH1750Sensor()
    : Sensor("BH1750", "lux")
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool BH1750Sensor::begin()
{
    // Initialize the BH1750 in continuous high-resolution mode
    // using I2C address 0x23.
    initialized = sensor.begin(
        BH1750::CONTINUOUS_HIGH_RES_MODE,
        0x23,
        &Wire
    );

    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool BH1750Sensor::read(Measurement& m)
{
    float value = sensor.readLightLevel();

    // A negative value indicates an invalid sensor reading
    if (value < 0)
    {
        return false;
    }

    m.luminosity = value;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* BH1750Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name, luminosity value and unit
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %.2f %s",
        getName(),
        m.luminosity,
        getUnit()
    );

    return buffer;
}