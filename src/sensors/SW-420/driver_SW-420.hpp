/**
 * @file    driver_SW-420.hpp
 * @brief   SW-420 vibration sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-02
 */

#pragma once

#include <Arduino.h>
#include "../sensor.hpp"


// ============================================================
// SW420Sensor class definition
// ============================================================

class SW420Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        int pin;  // GPIO pin connected to the SW-420 digital output

        volatile bool vibrationDetected = false; // Indicates whether a vibration event has been detected since the last read

        /**
         * @brief Interrupt service routine for vibration detection.
         * @param arg Pointer to the SW420Sensor instance that triggered
         *            the interrupt.
         */
        static void IRAM_ATTR handleInterrupt(void* arg);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for SW420Sensor.
         * @param pin GPIO pin connected to the SW-420 digital output.
         */
        SW420Sensor(int pin);

        /**
         * @brief Initialize the SW-420 sensor.
         * @return true if the selected GPIO supports interrupts and the
         *         interrupt was successfully configured, false otherwise.
         */
        bool begin() override;

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
    : Sensor("SW-420"),
      pin(pin)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool SW420Sensor::begin()
{
    // Configure the SW-420 output as a digital input
    pinMode(pin, INPUT);

    // Check whether the selected GPIO supports interrupts
    int interruptPin = digitalPinToInterrupt(pin);

    if (interruptPin == NOT_AN_INTERRUPT)
    {
        initialized = false;
    }
    else
    {
        // Trigger the interrupt when a vibration event is detected
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
// Interrupt Handler
// ─────────────────────────────────────────────────────────────────────────────

inline void IRAM_ATTR SW420Sensor::handleInterrupt(void* arg)
{
    // Recover the sensor instance associated with the interrupt
    SW420Sensor* sensor =
        static_cast<SW420Sensor*>(arg);

    // Store the vibration event for processing during the next read()
    sensor->vibrationDetected = true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool SW420Sensor::read(Measurement& m)
{
    // Copy the vibration event into the Measurement structure
    m.vibration = vibrationDetected;

    // Reset the event flag after it has been consumed
    vibrationDetected = false;

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