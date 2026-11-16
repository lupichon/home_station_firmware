/**
 * @file    led.hpp
 * @brief   Single PWM-driven LED on one GPIO pin.
 * @author  Lucas
 * @date    2026-11-10
 */

#pragma once

#include <Arduino.h>

class LED
{
    public:
        /**
         * @param pin     GPIO pin connected to the LED.
         * @param channel LEDC (PWM) channel used for this pin.
         */
        LED(uint8_t pin, uint8_t channel);

        /** @brief Configure the PWM channel and attach the pin. */
        void begin();

        /** @brief Set the LED intensity from 0 (off) to 255 (full). 
         * @param value Intensity value from 0 to 255.
         */
        void setIntensity(uint8_t value);

        /** @brief Turn the LED on at full intensity. */
        void on();

        /** @brief Turn the LED off. */
        void off();

        /** @brief Last intensity written to the LED. */
        uint8_t getIntensity() const;

    private:
        static constexpr uint32_t FREQUENCY_HZ    = 5000;
        static constexpr uint8_t  RESOLUTION_BITS = 8;

        uint8_t pin;
        uint8_t channel;
        uint8_t current = 0;
};

// ============================================================
// Implementations of LED methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
inline LED::LED(uint8_t pin, uint8_t channel) : pin(pin), channel(channel) {}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void LED::begin()
{
    ledcSetup(channel, FREQUENCY_HZ, RESOLUTION_BITS);
    ledcAttachPin(pin, channel);
    setIntensity(0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────

inline void LED::setIntensity(uint8_t value)
{
    current = value;
    ledcWrite(channel, value);
}

inline uint8_t LED::getIntensity() const
{
    return current;
}

// ─────────────────────────────────────────────────────────────────────────────
// Control
// ─────────────────────────────────────────────────────────────────────────────

inline void LED::off()
{
    setIntensity(0);
}

inline void LED::on()
{
    setIntensity(255);
}
