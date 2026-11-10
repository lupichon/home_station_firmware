/**
 * @file    driver_HC-SR501.hpp
 * @brief   HC-SR501 PIR motion sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-23
 */

#pragma once

#include <Arduino.h>
#include "../interrupt_sensor.hpp"


// ============================================================
// HCSR501Sensor class definition
// ============================================================

class HCSR501Sensor : public InterruptSensor
{
    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for HCSR501Sensor.
         * @param pin GPIO pin connected to the HC-SR501 digital output.
         */
        HCSR501Sensor(int pin);

        /**
         * @brief Read and consume the current motion detection event.
         * @param m Measurement object that will receive the motion state.
         * @return true after the motion state has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current motion detection state for display.
         * @param m Measurement containing the motion detection state.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of HCSR501Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline HCSR501Sensor::HCSR501Sensor(int pin)
    : InterruptSensor("HC-SR501", pin, HIGH) // Output goes HIGH when motion is detected
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool HCSR501Sensor::read(Measurement& m)
{
    // Copy the motion event into the Measurement structure
    m.motion = consumeEvent();

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* HCSR501Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and motion detection status
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.motion ? "Motion detected" : "No motion"
    );

    return buffer;
}