/**
 * @file    driver.hpp
 * @brief   Low-level BH1750 ambient light sensor driver (raw values only).
 * @author  Lucas Pichon
 * @date    2026-07-16
 */

#pragma once

#include <BH1750.h>
#include <Wire.h>


// ============================================================
// BH1750Driver class definition
// ============================================================

/**
 * @brief Hardware access to a BH1750 on the I2C bus, with no application dependency.
 *
 * The chip operates in one-time high-resolution mode.
 */
class BH1750Driver
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        static constexpr uint8_t I2C_ADDRESS = 0x23;

        BH1750 chip;  // Vendor library instance

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Initialize the chip.
         * @return true if the chip answered, false otherwise.
         */
        bool begin();

        /**
         * @brief Trigger one conversion and read the result.
         * @param lux Receives the light level in lux.
         * @return true if a valid value was read, false otherwise.
         */
        bool readLux(float& lux);
};


// ============================================================
// Implementations of BH1750Driver methods
// ============================================================

inline bool BH1750Driver::begin()
{
    return chip.begin(BH1750::ONE_TIME_HIGH_RES_MODE, I2C_ADDRESS, &Wire);
}

inline bool BH1750Driver::readLux(float& lux)
{
    // Trigger one measurement: the chip powers down by itself afterwards
    if (!chip.configure(BH1750::ONE_TIME_HIGH_RES_MODE))
    {
        return false;
    }

    float value = chip.readLightLevel();

    // A negative value indicates an invalid reading
    if (value < 0)
    {
        return false;
    }

    lux = value;

    return true;
}
