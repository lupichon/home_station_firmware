/**
 * @file    driver.hpp
 * @brief   Low-level SCD41 CO2, temperature and humidity driver (raw values only).
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>


// ============================================================
// SCD41Driver class definition
// ============================================================

class SCD41Driver
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        static constexpr uint8_t I2C_ADDRESS = 0x62;

        SensirionI2cScd4x chip;  // Vendor library instance

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Reset the chip and start low-power periodic measurements.
         * @return true if periodic measurement mode was started, false otherwise.
         */
        bool begin();

        /**
         * @brief Check whether a new measurement is available.
         * @param ready Receives the data-ready status.
         * @return true if the status was read, false on a bus error.
         */
        bool isDataReady(bool& ready);

        /**
         * @brief Read the latest measurement.
         * @return true if the measurement was read, false on a bus error.
         */
        bool read(uint16_t& co2, float& temperature, float& humidity);
};


// ============================================================
// Implementations of SCD41Driver methods
// ============================================================

inline bool SCD41Driver::begin()
{
    chip.begin(Wire, I2C_ADDRESS);

    // Reset the measurement state before starting periodic measurements
    chip.wakeUp();
    chip.stopPeriodicMeasurement();
    delay(500);
    chip.reinit();

    // Allow the chip to complete its initialization
    delay(1000);

    return chip.startLowPowerPeriodicMeasurement() == 0;
}

inline bool SCD41Driver::isDataReady(bool& ready)
{
    return chip.getDataReadyStatus(ready) == 0;
}

inline bool SCD41Driver::read(uint16_t& co2, float& temperature, float& humidity)
{
    return chip.readMeasurement(co2, temperature, humidity) == 0;
}
