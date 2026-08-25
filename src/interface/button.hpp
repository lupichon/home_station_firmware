/**
 * @file    button.hpp
 * @brief   Debounced push-button interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-28
 */

#pragma once

#include <Arduino.h>


// ============================================================
// Button class definition
// ============================================================

class Button
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        uint8_t pin;              // GPIO pin connected to the button

        bool currentState;        // Debounced current state of the button (HIGH/LOW)
        bool lastState;           // Last raw reading of the button pin
        bool pressed;             // Latched flag set on a press event, cleared by wasPressed()

        unsigned long lastDebounceTime;  // Timestamp of the last observed state change

        static constexpr unsigned long DEBOUNCE_TIME = 50;  // Debounce delay in milliseconds

    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for Button.
         * @param pin GPIO pin connected to the button.
         */
        Button(uint8_t pin);

        /**
         * @brief Configure the GPIO pin and reset the internal state.
         */
        void begin();

        /**
         * @brief Poll the button pin and update its debounced state.
         *        Must be called regularly (e.g. in the main loop).
         */
        void update();

        /**
         * @brief Check whether a press event has occurred since the last call.
         * @return true if the button was pressed, false otherwise. The
         *         internal flag is cleared after being read.
         */
        bool wasPressed();

        /**
         * @brief Check whether the button is currently held down.
         * @return true if the button is currently pressed, false otherwise.
         */
        bool isHeld() const;
};

// ============================================================
// implementations of Button methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline Button::Button(uint8_t pin)
    : pin(pin),
      currentState(HIGH),
      lastState(HIGH),
      pressed(false),
      lastDebounceTime(0)
{

}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void Button::begin()
{
    // Configure the pin with internal pull-up
    pinMode(pin, INPUT_PULLUP);

    currentState = HIGH;
    lastState = HIGH;
    pressed = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// State Update
// ─────────────────────────────────────────────────────────────────────────────

inline void Button::update()
{
    bool reading = digitalRead(pin);

    if (reading != lastState)   // Raw state changed, restart the debounce timer
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME)  // State has been stable long enough
    {
        if (reading != currentState)
        {
            currentState = reading;

            // Bouton pressé
            if (currentState == LOW)
            {
                pressed = true;
            }
        }
    }

    lastState = reading;
}

// ─────────────────────────────────────────────────────────────────────────────
// Press Detection
// ─────────────────────────────────────────────────────────────────────────────

inline bool Button::wasPressed()
{
    if (pressed)
    {
        pressed = false;
        return true;
    }

    return false;
}

inline bool Button::isHeld() const
{
    return currentState == LOW;
}