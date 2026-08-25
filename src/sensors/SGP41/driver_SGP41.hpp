/**
 * @file    driver_SGP41.hpp
 * @brief   SGP41 VOC and NOx air quality sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-07
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CSgp41.h>
#include <NOxGasIndexAlgorithm.h>
#include <VOCGasIndexAlgorithm.h>
#include "../sensor.hpp"


// ============================================================
// SGP41Sensor class definition
// ============================================================

class SGP41Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        SensirionI2CSgp41 sensor;  // SGP41 sensor driver instance

        VOCGasIndexAlgorithm vocAlgorithm; // Algorithm used to calculate the VOC gas index
        NOxGasIndexAlgorithm noxAlgorithm; // Algorithm used to calculate the NOx gas index

        unsigned long lastReadTime = 0; // Timestamp of the last measurement attempt

        static constexpr unsigned long READ_INTERVAL_MS = 1000; // Minimum interval between two SGP41 measurements

        uint16_t lastVocIndex = NAN; // Last successfully calculated VOC index
        uint16_t lastNoxIndex = NAN; // Last successfully calculated NOx index

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for SGP41Sensor.
         *
         * Initializes the sensor with the name "SGP41".
         */
        SGP41Sensor();

        /**
         * @brief Initialize and condition the SGP41 sensor.
         * @return true if the final conditioning operation succeeds,
         *         false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read and process the current VOC and NOx measurements.
         * @param m Measurement object containing the environmental values
         *          and receiving the VOC and NOx indexes.
         * @return true if valid indexes were calculated or cached values
         *         were returned, false if the sensor is not initialized
         *         or a measurement error occurred.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current VOC and NOx indexes for display.
         * @param m Measurement containing the VOC and NOx indexes.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of SGP41Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline SGP41Sensor::SGP41Sensor()
    : Sensor("SGP41")
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization and Conditioning
// ─────────────────────────────────────────────────────────────────────────────

inline bool SGP41Sensor::begin()
{
    // Initialize the SGP41 using the I2C bus
    sensor.begin(Wire);

    uint16_t error;
    uint16_t srawVoc;

    // Perform the sensor conditioning sequence
    for (uint16_t i = 0; i < 10; i++)
    {
        error = sensor.executeConditioning(
            0x8000,
            0x6666,
            srawVoc
        );

        delay(1000);
    }

    // The sensor is considered initialized if the final
    // conditioning operation completed successfully
    initialized = (error == 0);

    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool SGP41Sensor::read(Measurement& m)
{
    // Do not attempt to read an uninitialized sensor
    if (!initialized)
    {
        return false;
    }

    unsigned long now = millis();

    // Limit measurements to one per second
    if (now - lastReadTime < READ_INTERVAL_MS)
    {
        // Return the most recent valid indexes
        m.vocIndex = lastVocIndex;
        m.noxIndex = lastNoxIndex;

        return true;
    }

    lastReadTime = now;

    // Default compensation values:
    // 50 % relative humidity and 25 °C
    uint16_t humComp  = 0x8000;
    uint16_t tempComp = 0x6666;

    // Use the current humidity value when available
    if (!isnan(m.humidity))
    {
        humComp = (uint16_t)(
            (m.humidity / 100.0) * 65535.0
        );
    }

    // Use the current temperature value when available
    if (!isnan(m.temperature))
    {
        tempComp = (uint16_t)(
            ((m.temperature + 45.0) / 175.0) * 65535.0
        );
    }

    uint16_t srawVoc;
    uint16_t srawNox;

    // Read the compensated raw VOC and NOx signals
    uint16_t error = sensor.measureRawSignals(
        humComp,
        tempComp,
        srawVoc,
        srawNox
    );

    if (error != 0)
    {
        return false;
    }

    // Process raw signals using Sensirion's gas index algorithms
    lastVocIndex = m.vocIndex =
        (uint16_t)vocAlgorithm.process(srawVoc);

    lastNoxIndex = m.noxIndex =
        (uint16_t)noxAlgorithm.process(srawNox);

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* SGP41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and air quality indexes
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: VOC=%u NOx=%u",
        getName(),
        m.vocIndex,
        m.noxIndex
    );

    return buffer;
}