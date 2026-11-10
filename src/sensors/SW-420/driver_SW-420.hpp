/**
 * @file    driver_SW-420.hpp
 * @brief   SW-420 vibration sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-02
 */

#pragma once

#include <Arduino.h>
#include "../interrupt_sensor.hpp"


// ============================================================
// SW420Sensor class definition
// ============================================================

class SW420Sensor : public InterruptSensor
{
    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for SW420Sensor.
         * @param pin GPIO pin connected to the SW-420 digital output.
         */
        SW420Sensor(int pin);

        /**
         * @brief Read and consume the current vibration detection event.
         * @param m Measurement object that will receive the vibration state.
         * @return true after the vibration state has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current vibration detection state for display.
         * @param m Measurement containing the vibration detection state.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of SW420Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline SW420Sensor::SW420Sensor(int pin)
    : InterruptSensor("SW-420", pin, HIGH) // Output goes HIGH when a vibration is detected
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool SW420Sensor::read(Measurement& m)
{
    // Copy the vibration event into the Measurement structure
    m.vibration = consumeEvent();

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* SW420Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and vibration detection status
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.vibration ? "Vibration detected"
                    : "No vibration"
    );

    return buffer;
}