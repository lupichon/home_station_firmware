/**
 * @file    driver.hpp
 * @brief   Low-level BMP280 barometric pressure driver (raw values only).
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <Wire.h>
#include <Adafruit_BMP280.h>


// ============================================================
// BMP280Driver class definition
// ============================================================

class BMP280Driver
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        static constexpr uint8_t I2C_ADDRESS = 0x76;

        Adafruit_BMP280 chip;  // Vendor library instance

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Initialize the chip in forced mode.
         * @return true if the chip answered, false otherwise.
         */
        bool begin();

        /**
         * @brief Trigger one measurement and read the pressure at the sensor location.
         * @param hPa Receives the station pressure in hPa.
         * @return true if the measurement succeeded, false otherwise.
         */
        bool readPressureHpa(float& hPa);
};


// ============================================================
// Implementations of BMP280Driver methods
// ============================================================

inline bool BMP280Driver::begin()
{
    if (!chip.begin(I2C_ADDRESS))
    {
        return false;
    }

    chip.setSampling(
        Adafruit_BMP280::MODE_FORCED,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::FILTER_OFF,
        Adafruit_BMP280::STANDBY_MS_1
    );

    return true;
}

inline bool BMP280Driver::readPressureHpa(float& hPa)
{
    if (!chip.takeForcedMeasurement())
    {
        return false;
    }

    hPa = chip.readPressure() / 100.0f;

    return true;
}
