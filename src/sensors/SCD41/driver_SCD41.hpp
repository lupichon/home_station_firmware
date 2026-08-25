/**
 * @file    driver_SCD41.hpp
 * @brief   SCD41 CO2, temperature and humidity sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include "../sensor.hpp"


// ============================================================
// SCD41Sensor class definition
// ============================================================

class SCD41Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        SensirionI2cScd4x sensor;  // SCD41 sensor driver instance

        float lastTemperature;      // Last successfully measured temperature
        float lastHumidity;         // Last successfully measured relative humidity
        uint16_t lastCO2;           // Last successfully measured CO2 concentration

        unsigned long lastSuccessfulReadTime; // Timestamp of the last successfully acquired measurement

        static constexpr unsigned long MAX_MEASUREMENT_AGE_MS = 30000; // Maximum age of a valid measurement (30 seconds)

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for SCD41Sensor.
         *
         * Initializes the sensor name and resets the cached measurement
         * values to undefined values.
         */
        SCD41Sensor();

        /**
         * @brief Initialize the SCD41 sensor.
         *
         * Configures the SCD41 on the I2C bus at address 0x62, resets
         * the sensor state and starts periodic measurements.
         *
         * @return true if periodic measurement mode was started successfully,
         *         false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read the latest SCD41 measurement.
         *
         * If new data is available, the CO2 concentration, temperature
         * and humidity values are read and cached.
         *
         * If no new data is available, the last successful measurement
         * is returned. If no successful measurement has been received
         * for more than 30 seconds, the read operation fails.
         *
         * @param m Measurement object that will receive the sensor data.
         * @return true if valid data was returned, false if an error occurred
         *         or no valid measurement has been available for too long.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current SCD41 measurements for display.
         *
         * The returned string contains the sensor name, CO2 concentration,
         * temperature and relative humidity.
         *
         * @param m Measurement containing the SCD41 values to display.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of SCD41Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline SCD41Sensor::SCD41Sensor()
    : Sensor("SCD41"),
      lastTemperature(NAN),
      lastHumidity(NAN),
      lastCO2(NAN),
      lastSuccessfulReadTime(millis())
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool SCD41Sensor::begin()
{
    // Initialize the SCD41 on the I2C bus at address 0x62
    sensor.begin(Wire, 0x62);

    // Reset the sensor measurement state before starting periodic measurements
    sensor.wakeUp();
    sensor.stopPeriodicMeasurement();
    sensor.reinit();

    // Allow the sensor to complete its initialization
    delay(1000);

    // Start periodic measurement mode
    int16_t error = sensor.startPeriodicMeasurement();

    initialized = (error == 0);

    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool SCD41Sensor::read(Measurement& m)
{
    bool dataReady = false;

    // Check whether a new measurement is available
    uint16_t error = sensor.getDataReadyStatus(dataReady);

    if (error != 0)
    {
        return false;
    }

    // ── No new measurement available ──
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

    // ── New measurement available ──
    uint16_t co2      = 0;
    float temperature = 0.0f;
    float humidity    = 0.0f;

    error = sensor.readMeasurement(co2, temperature, humidity);

    if (error != 0)
    {
        return false;
    }

    // Store the new measurement and update the cached values
    m.co2         = lastCO2         = co2;
    m.temperature = lastTemperature = temperature;
    m.humidity    = lastHumidity    = humidity;

    // Record the time of the successful measurement
    lastSuccessfulReadTime = millis();

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* SCD41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[128];

    // Format CO2, temperature and humidity values for display
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