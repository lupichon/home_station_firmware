/**
 * @file    driver.hpp
 * @brief   Low-level SGP41 VOC/NOx driver (raw signals only, no gas index processing).
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <math.h>
#include <Wire.h>
#include <SensirionI2CSgp41.h>


// ============================================================
// SGP41Driver class definition
// ============================================================

class SGP41Driver
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        SensirionI2CSgp41 chip;  // Vendor library instance

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Initialize the chip and run the conditioning sequence (blocks about 10 s).
         * @return true if the last conditioning step succeeded, false otherwise.
         */
        bool begin();

        /**
         * @brief Read the raw VOC and NOx signals, compensated by humidity and temperature.
         * @param humidity    Relative humidity in percent, NAN to use 50 %.
         * @param temperature Temperature in degrees Celsius, NAN to use 25 degrees.
         * @param rawVoc      Receives the raw VOC signal.
         * @param rawNox      Receives the raw NOx signal.
         * @return true if the signals were read, false on a bus error.
         */
        bool readRaw(float humidity, float temperature, uint16_t& rawVoc, uint16_t& rawNox);
};


// ============================================================
// Implementations of SGP41Driver methods
// ============================================================

inline bool SGP41Driver::begin()
{
    chip.begin(Wire);

    uint16_t error = 0;
    uint16_t rawVoc;

    for (uint16_t i = 0; i < 10; i++)
    {
        error = chip.executeConditioning(0x8000, 0x6666, rawVoc);

        delay(1000);
    }

    return error == 0;
}

inline bool SGP41Driver::readRaw(float humidity, float temperature, uint16_t& rawVoc, uint16_t& rawNox)
{
    // Default compensation values: 50 % relative humidity and 25 degrees Celsius
    uint16_t humComp  = 0x8000;
    uint16_t tempComp = 0x6666;

    if (!isnan(humidity))
    {
        humComp = (uint16_t)((humidity / 100.0) * 65535.0);
    }

    if (!isnan(temperature))
    {
        tempComp = (uint16_t)(((temperature + 45.0) / 175.0) * 65535.0);
    }

    return chip.measureRawSignals(humComp, tempComp, rawVoc, rawNox) == 0;
}
