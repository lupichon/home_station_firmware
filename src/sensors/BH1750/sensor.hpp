/**
 * @file    sensor.hpp
 * @brief   BH1750 light sensor as seen by the application (maps the driver to Measurement).
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stdio.h>
#include "../sensor.hpp"
#include "driver.hpp"


// ============================================================
// BH1750Sensor class definition
// ============================================================

class BH1750Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        BH1750Driver driver;  // Low-level access to the chip

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for BH1750Sensor.
         */
        BH1750Sensor();

        /**
         * @brief Initialize the underlying driver.
         * @return true if the sensor was initialized successfully, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read the ambient light level into the measurement.
         * @param m Measurement that receives the luminosity.
         * @return true if a valid value was read, false otherwise.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the luminosity of a measurement for display.
         * @param m Measurement containing the luminosity.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of BH1750Sensor methods
// ============================================================

inline BH1750Sensor::BH1750Sensor()
    : Sensor("BH1750", "lux", BH1750_BIT)
{
}

inline bool BH1750Sensor::begin()
{
    initialized = driver.begin();

    return initialized;
}

inline bool BH1750Sensor::read(Measurement& m)
{
    float lux;

    if (!initialized || !driver.readLux(lux))
    {
        return false;
    }

    m.luminosity = lux;

    return true;
}

inline const char* BH1750Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s: %.2f %s", getName(), m.luminosity, getUnit());

    return buffer;
}
