/**
 * @file    sensor.hpp
 * @brief   SCD41 CO2, temperature and humidity sensor as seen by the application.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include "../sensor.hpp"
#include "driver.hpp"


// ============================================================
// SCD41Sensor class definition
// ============================================================

class SCD41Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        SCD41Driver driver;  // Low-level access to the chip

        float lastTemperature;      // Last successfully measured temperature
        float lastHumidity;         // Last successfully measured relative humidity
        uint16_t lastCO2;           // Last successfully measured CO2 concentration

        unsigned long lastSuccessfulReadTime; // Timestamp of the last successfully acquired measurement

        static constexpr unsigned long MAX_MEASUREMENT_AGE_MS = 60000; // Maximum age of a valid measurement

    // ── Public interface ────────────────────────────────────────────────────
    public:

        SCD41Sensor();

        /**
         * @brief Initialize the underlying driver.
         * @return true if periodic measurement mode was started, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read the latest SCD41 measurement.
         *
         * If no new data is available, the last successful measurement is returned.
         * If no successful measurement has been received for more than
         * MAX_MEASUREMENT_AGE_MS, the read fails.
         *
         * @param m Measurement that receives CO2, temperature and humidity.
         * @return true if valid data was returned, false otherwise.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Last measured temperature, used by other sensors for compensation.
         * @return Temperature in degrees Celsius, or NAN if none has been measured yet.
         */
        float temperature() const;

        /**
         * @brief Last measured relative humidity, used by other sensors for compensation.
         * @return Relative humidity in percent, or NAN if none has been measured yet.
         */
        float humidity() const;

        /**
         * @brief Format the SCD41 values of a measurement for display.
         * @param m Measurement containing the SCD41 values.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of SCD41Sensor methods
// ============================================================

inline SCD41Sensor::SCD41Sensor()
    : Sensor("SCD41", "", SCD41_BIT),
      lastTemperature(NAN),
      lastHumidity(NAN),
      lastCO2(0),
      lastSuccessfulReadTime(millis())
{
}

inline bool SCD41Sensor::begin()
{
    initialized = driver.begin();

    return initialized;
}

inline bool SCD41Sensor::read(Measurement& m)
{
    bool dataReady = false;

    if (!driver.isDataReady(dataReady))
    {
        return false;
    }

    if (!dataReady)
    {
        // Fail if the sensor has not provided valid data for too long
        if (millis() - lastSuccessfulReadTime > MAX_MEASUREMENT_AGE_MS)
        {
            return false;
        }

        // Return the most recent valid measurement
        m.co2         = lastCO2;
        m.temperature = lastTemperature;
        m.humidity    = lastHumidity;

        return true;
    }

    uint16_t co2      = 0;
    float temperature = 0.0f;
    float humidity    = 0.0f;

    if (!driver.read(co2, temperature, humidity))
    {
        return false;
    }

    m.co2         = lastCO2         = co2;
    m.temperature = lastTemperature = temperature;
    m.humidity    = lastHumidity    = humidity;

    lastSuccessfulReadTime = millis();

    return true;
}

inline float SCD41Sensor::temperature() const
{
    return lastTemperature;
}

inline float SCD41Sensor::humidity() const
{
    return lastHumidity;
}

inline const char* SCD41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[128];

    snprintf(
        buffer,
        sizeof(buffer),
        "%s: CO2=%u ppm | Temp=%.2f C | Hum=%.2f %%",
        getName(),
        m.co2,
        m.temperature,
        m.humidity
    );

    return buffer;
}
