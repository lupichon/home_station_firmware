/**
 * @file    interrupt_sensor.hpp
 * @brief   Base class for digital sensors reporting events through a GPIO interrupt.
 * @author  Lucas Pichon
 * @date    2026-11-10
 */

#pragma once

#include <Arduino.h>
#include "sensor.hpp"


// ============================================================
// InterruptSensor class definition
// ============================================================

/**
 * @brief Base class for digital sensors whose output goes to an "active" level
 *        when something is detected (motion, vibration, obstacle...).
 *
 * An interrupt is attached on the active edge, so that events shorter than the
 * read interval are not lost. A read reports the sensor as active if an event
 * occurred since the last read, or if the output is still at its active level.
 */
class InterruptSensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        int pin;          // GPIO pin connected to the sensor digital output
        int activeLevel;  // Output level when the sensor detects something (HIGH or LOW)

        volatile bool eventDetected = false; // Indicates whether an event has been detected since the last read

        /**
         * @brief Interrupt service routine for event detection.
         * @param arg Pointer to the InterruptSensor instance that triggered
         *            the interrupt.
         */
        static void IRAM_ATTR handleInterrupt(void* arg);

    // ── Protected interface (for derived sensors) ───────────────────────────
    protected:

        /**
         * @brief Constructor for InterruptSensor.
         * @param name        Sensor name.
         * @param pin         GPIO pin connected to the sensor digital output.
         * @param activeLevel Output level when something is detected (HIGH or LOW).
         */
        InterruptSensor(const char* name, int pin, int activeLevel);

        /**
         * @brief Report and consume the current detection state.
         *
         * Returns true if an event occurred since the previous call, or if the
         * output is still at its active level, then clears the event flag.
         *
         * @return true if the sensor is considered active.
         */
        bool consumeEvent();

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Initialize the sensor.
         *
         * Configures the GPIO as an input and attaches an interrupt on the
         * active edge (RISING if active HIGH, FALLING if active LOW).
         *
         * @return true if the GPIO supports interrupts and the interrupt
         *         was configured, false otherwise.
         */
        bool begin() override;
};


// ============================================================
// Implementations of InterruptSensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline InterruptSensor::InterruptSensor(const char* name, int pin, int activeLevel)
    : Sensor(name),
      pin(pin),
      activeLevel(activeLevel)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool InterruptSensor::begin()
{
    // Configure the sensor output as a digital input
    pinMode(pin, INPUT);

    // Check whether the selected GPIO supports interrupts
    int interruptPin = digitalPinToInterrupt(pin);

    if (interruptPin == NOT_AN_INTERRUPT)
    {
        initialized = false;
    }
    else
    {
        // Trigger the interrupt on the edge leading to the active level
        attachInterruptArg(
            interruptPin,
            handleInterrupt,
            this,
            (activeLevel == HIGH) ? RISING : FALLING
        );

        initialized = true;
    }

    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Interrupt Handler
// ─────────────────────────────────────────────────────────────────────────────

inline void IRAM_ATTR InterruptSensor::handleInterrupt(void* arg)
{
    // Recover the sensor instance associated with the interrupt
    InterruptSensor* sensor = static_cast<InterruptSensor*>(arg);

    // Store the event for processing during the next read()
    sensor->eventDetected = true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Event Consumption
// ─────────────────────────────────────────────────────────────────────────────

inline bool InterruptSensor::consumeEvent()
{
    // Event seen since the last read, or output still at its active level
    bool active = eventDetected || (digitalRead(pin) == activeLevel);

    // Reset the event flag after it has been consumed
    eventDetected = false;

    return active;
}