/**
 * @file    analog_input.hpp
 * @brief   Low-level 12-bit ADC input on a GPIO pin.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>


// ============================================================
// AnalogInput class definition
// ============================================================

class AnalogInput
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        uint8_t pin;  // Analog GPIO pin

    // ── Public interface ────────────────────────────────────────────────────
    public:

        static constexpr int MAX_VALUE = 4095;  // Full-scale value of the 12-bit ADC

        explicit AnalogInput(uint8_t pin) : pin(pin) {}

        /**
         * @brief Configure the pin as an analog input with 12-bit resolution.
         */
        void begin()
        {
            pinMode(pin, INPUT);
            analogReadResolution(12);
        }

        /**
         * @brief Read the raw ADC value (0 to MAX_VALUE).
         */
        uint16_t read() const
        {
            return analogRead(pin);
        }
};
