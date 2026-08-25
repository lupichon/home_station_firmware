/**
 * @file    driver_HC-SR501.hpp
 * @brief   HC-SR501 PIR motion sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-23
 */

#pragma once

#include <Arduino.h>
#include "../sensor.hpp"


// ============================================================
// HCSR501Sensor class definition
// ============================================================

class HCSR501Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        int pin;  // GPIO pin connected to the HC-SR501 output

        volatile bool motionDetected = false; // Indicates whether motion has been detected since the last read

        /**
         * @brief Interrupt service routine for motion detection.
         *
         * This function is called when the HC-SR501 output changes to HIGH.
         * It sets the motion detection flag associated with the sensor.
         *
         * @param arg Pointer to the HCSR501Sensor instance that triggered
         *            the interrupt.
         */
        static void IRAM_ATTR handleInterrupt(void* arg);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for HCSR501Sensor.
         * @param pin GPIO pin connected to the HC-SR501 digital output.
         */
        HCSR501Sensor(int pin);

        /**
         * @brief Initialize the HC-SR501 sensor.
         *
         * Configures the sensor GPIO as a digital input and attaches an
         * interrupt triggered on the rising edge of the sensor output.
         *
         * @return true if the GPIO supports interrupts and the interrupt
         *         was configured, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Read and consume the current motion detection event.
         *
         * Copies the motion detection flag into the Measurement object,
         * then clears the internal flag so that the same event is not
         * reported again on the next read.
         *
         * @param m Measurement object that will receive the motion state.
         * @return true after the motion state has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current motion detection state for display.
         *
         * The returned string indicates whether motion has been detected
         * since the previous sensor read.
         *
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
    : Sensor("HC-SR501"),
      pin(pin)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool HCSR501Sensor::begin()
{
    // Configure the HC-SR501 output as a digital input
    pinMode(pin, INPUT);

    // Check whether the selected GPIO supports interrupts
    int interruptPin = digitalPinToInterrupt(pin);

    if (interruptPin == NOT_AN_INTERRUPT)
    {
        initialized = false;
    }
    else
    {
        // Trigger the interrupt when motion is detected
        attachInterruptArg(
            interruptPin,
            handleInterrupt,
            this,
            RISING
        );

        initialized = true;
    }

    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool HCSR501Sensor::read(Measurement& m)
{
    // Copy the motion event into the Measurement structure
    m.motion = motionDetected;

    // Reset the event flag after it has been consumed
    motionDetected = false;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Interrupt Handler
// ─────────────────────────────────────────────────────────────────────────────

inline void IRAM_ATTR HCSR501Sensor::handleInterrupt(void* arg)
{
    // Recover the sensor instance associated with the interrupt
    HCSR501Sensor* sensor = static_cast<HCSR501Sensor*>(arg);

    // Store the motion event for processing during the next read()
    sensor->motionDetected = true;
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