/**
 * @file    driver_FC-51.hpp
 * @brief   FC-51 infrared obstacle detection sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-27
 */

#pragma once

#include <Arduino.h>
#include "../interrupt_sensor.hpp"


// ============================================================
// FC51Sensor class definition
// ============================================================

class FC51Sensor : public InterruptSensor
{
    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for FC51Sensor.
         * @param pin GPIO pin connected to the FC-51 sensor output.
         */
        FC51Sensor(int pin);

        /**
         * @brief Read and consume the current obstacle detection event.
         * @param m Measurement object that will receive the obstacle state.
         * @return true after the sensor state has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current obstacle detection state for display.
         * @param m Measurement containing the obstacle detection state.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of FC51Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline FC51Sensor::FC51Sensor(int pin)
    : InterruptSensor("FC-51", pin, LOW) // Output goes LOW when an obstacle is detected
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool FC51Sensor::read(Measurement& m)
{
    // Copy the obstacle event into the Measurement structure
    m.obstacle = consumeEvent();

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* FC51Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and obstacle detection status
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.obstacle ? "Obstacle detected" : "No obstacle"
    );

    return buffer;
}