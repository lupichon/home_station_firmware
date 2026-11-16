/**
 * @file    sensor.hpp
 * @brief   SGP41 VOC/NOx sensor as seen by the application (gas index algorithms, SCD41 compensation).
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <stdio.h>
#include <NOxGasIndexAlgorithm.h>
#include <VOCGasIndexAlgorithm.h>
#include "../sensor.hpp"
#include "../SCD41/sensor.hpp"
#include "driver.hpp"


// ============================================================
// SGP41Sensor class definition
// ============================================================

/**
 * @brief Gas index algorithms assume exactly one sample per second, so the sensor
 *        is sampled from update() and read() only returns the latest indexes.
 */
class SGP41Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        SGP41Driver driver;         // Low-level access to the chip
        const SCD41Sensor& climate; // Source of temperature and humidity compensation

        VOCGasIndexAlgorithm vocAlgorithm; // Algorithm used to calculate the VOC gas index
        NOxGasIndexAlgorithm noxAlgorithm; // Algorithm used to calculate the NOx gas index

        unsigned long lastSampleMs = 0; // Timestamp of the last sample

        static constexpr unsigned long SAMPLE_INTERVAL_MS = 1000; // Sampling period expected by the algorithms

        bool lastSampleOk = true;  // Whether the last sample succeeded
        uint16_t lastVocIndex = 0; // Last calculated VOC index
        uint16_t lastNoxIndex = 0; // Last calculated NOx index

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @param climate Sensor providing the temperature and humidity used for compensation.
         */
        explicit SGP41Sensor(const SCD41Sensor& climate);

        /**
         * @brief Initialize and condition the sensor.
         * @return true if the conditioning succeeded, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Take one sample per SAMPLE_INTERVAL_MS and update the gas indexes.
         */
        void update() override;

        /**
         * @brief Store the latest VOC and NOx indexes in the measurement.
         * @param m Measurement that receives the indexes.
         * @return true if the last sample succeeded, false otherwise.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the VOC and NOx indexes of a measurement for display.
         * @param m Measurement containing the indexes.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of SGP41Sensor methods
// ============================================================

inline SGP41Sensor::SGP41Sensor(const SCD41Sensor& climate)
    : Sensor("SGP41", "", SGP41_BIT),
      climate(climate)
{
}

inline bool SGP41Sensor::begin()
{
    initialized = driver.begin();
    lastSampleOk = initialized;
    lastSampleMs = millis();

    return initialized;
}

inline void SGP41Sensor::update()
{
    unsigned long now = millis();

    if (now - lastSampleMs < SAMPLE_INTERVAL_MS)
    {
        return;
    }

    lastSampleMs = now;

    uint16_t rawVoc;
    uint16_t rawNox;

    lastSampleOk = driver.readRaw(climate.humidity(), climate.temperature(), rawVoc, rawNox);

    if (lastSampleOk)
    {
        lastVocIndex = (uint16_t)vocAlgorithm.process(rawVoc);
        lastNoxIndex = (uint16_t)noxAlgorithm.process(rawNox);
    }
}

inline bool SGP41Sensor::read(Measurement& m)
{
    if (!initialized || !lastSampleOk)
    {
        return false;
    }

    m.vocIndex = lastVocIndex;
    m.noxIndex = lastNoxIndex;

    return true;
}

inline const char* SGP41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s: VOC=%u NOx=%u", getName(), m.vocIndex, m.noxIndex);

    return buffer;
}
