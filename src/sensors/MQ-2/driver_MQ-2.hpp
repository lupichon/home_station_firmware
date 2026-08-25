/**
 * @file    driver_MQ-2.hpp
 * @brief   MQ-2 analog gas sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-02
 */

#pragma once

#include <Arduino.h>
#include "../sensor.hpp"


// ============================================================
// MQ2Sensor class definition
// ============================================================

class MQ2Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        uint8_t pin;  // Analog GPIO pin connected to the MQ-2 output

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for MQ2Sensor.
         * @param pin Analog GPIO pin connected to the MQ-2 output.
         */
        MQ2Sensor(uint8_t pin);

        /**
         * @brief Initialize the MQ-2 sensor.
         *
         * Configures the sensor pin as an analog input and sets the ADC
         * resolution to 12 bits.
         *
         * @return true after the sensor has been initialized.
         */
        bool begin() override;

        /**
         * @brief Read the raw gas sensor value.
         *
         * Reads the analog output of the MQ-2 sensor and stores the
         * raw ADC value in the Measurement object.
         *
         * @param m Measurement object that will receive the raw gas value.
         * @return true after the measurement has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the raw gas sensor value for display.
         *
         * The returned string contains the sensor name followed by
         * the raw ADC value.
         *
         * @param m Measurement containing the raw gas sensor value.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of MQ2Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline MQ2Sensor::MQ2Sensor(uint8_t pin)
    : Sensor("MQ-2"),
      pin(pin)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool MQ2Sensor::begin()
{
    // Configure the MQ-2 output as an analog input
    pinMode(pin, INPUT);

    // Configure the ADC for 12-bit resolution (0-4095)
    analogReadResolution(12);

    initialized = true;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool MQ2Sensor::read(Measurement& m)
{
    // Read the raw ADC value from the MQ-2 sensor
    uint16_t gasValue = analogRead(pin);

    // Store the raw value in the Measurement structure
    m.gasRaw = gasValue;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* MQ2Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and raw gas sensor value
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %d",
        getName(),
        m.gasRaw
    );

    return buffer;
}