/**
 * @file    interrupt_input.hpp
 * @brief   Low-level digital input that latches an edge through a GPIO interrupt.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>


// ============================================================
// InterruptInput class definition
// ============================================================

/**
 * @brief Digital input whose "active" level signals an event (motion, vibration...).
 *
 * An interrupt is attached on the active edge, so that events shorter than the
 * polling interval are not lost.
 */
class InterruptInput
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        int pin;          // GPIO pin connected to the digital output
        int activeLevel;  // Output level signalling an event (HIGH or LOW)
        bool useLevel;    // Whether a pin still at its active level counts as an event

        volatile bool eventDetected = false;  // Set by the interrupt, cleared by consumeEvent()

        static void IRAM_ATTR handleInterrupt(void* arg);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @param pin         GPIO pin connected to the digital output.
         * @param activeLevel Output level signalling an event (HIGH or LOW).
         * @param useLevel    Also report an event while the pin stays at its active level.
         */
        InterruptInput(int pin, int activeLevel, bool useLevel);

        /**
         * @brief Configure the GPIO and attach the interrupt on the active edge.
         * @return true if the GPIO supports interrupts, false otherwise.
         */
        bool begin();

        /**
         * @brief Report and clear the event state.
         * @return true if an event occurred since the previous call, or if the
         *         output is still at its active level (when useLevel is set).
         */
        bool consumeEvent();
};


// ============================================================
// Implementations of InterruptInput methods
// ============================================================

inline InterruptInput::InterruptInput(int pin, int activeLevel, bool useLevel)
    : pin(pin),
      activeLevel(activeLevel),
      useLevel(useLevel)
{
}

inline bool InterruptInput::begin()
{
    pinMode(pin, INPUT);

    int interruptPin = digitalPinToInterrupt(pin);

    if (interruptPin == NOT_AN_INTERRUPT)
    {
        return false;
    }

    // Trigger on the edge leading to the active level
    attachInterruptArg(
        interruptPin,
        handleInterrupt,
        this,
        (activeLevel == HIGH) ? RISING : FALLING
    );

    return true;
}

inline void IRAM_ATTR InterruptInput::handleInterrupt(void* arg)
{
    static_cast<InterruptInput*>(arg)->eventDetected = true;
}

inline bool InterruptInput::consumeEvent()
{
    bool active = eventDetected || (useLevel && digitalRead(pin) == activeLevel);

    eventDetected = false;

    return active;
}
