/**
 * @file    rgb_led.hpp
 * @brief   RGB LED built from three single LEDs.
 * @author  Lucas
 * @date    2026-11-10
 */

#pragma once

#include "led.hpp"

/**
 * @brief Represents an RGB color using 8-bit intensity values.
 */
struct RGB
{
    uint8_t r, g, b;
};

/**
 * @brief Predefined RGB colors.
 */
namespace LEDColor
{
    static constexpr RGB OFF     = {  0,   0,   0};
    static constexpr RGB BLUE    = {  0,   0, 255};
    static constexpr RGB MAGENTA = {255,   0, 255};
    static constexpr RGB GREEN   = {  0, 255,   0};
    static constexpr RGB ORANGE  = {255,  10,   0};
    static constexpr RGB PURPLE  = {128,   0,  50};
    static constexpr RGB RED     = {255,   0,   0};
    static constexpr RGB CYAN    = {  0,  90, 255};
    static constexpr RGB YELLOW  = {255,  40,   0};
    static constexpr RGB WHITE   = {255, 255, 255};
    static constexpr RGB TEAL    = {  0, 128, 128};
    static constexpr RGB BROWN   = {255,  30,  20};
}

class RgbLED
{
    public:
        /**
         * @param red   Red channel LED.
         * @param green Green channel LED.
         * @param blue  Blue channel LED.
         */
        RgbLED(LED& red, LED& green, LED& blue);

        /** @brief Initialize the three underlying LEDs. */
        void begin();

        /** @brief Set the output color from three channel intensities.
         * @param r Red channel intensity from 0 to 255.
         * @param g Green channel intensity from 0 to 255.
         * @param b Blue channel intensity from 0 to 255.
         */
        void setColor(uint8_t r, uint8_t g, uint8_t b);

        /** @brief Set the output color from an RGB value. 
         *  @param color RGB color value.
        */
        void setColor(const RGB& color);

        /** @brief Turn all channels off. */
        void off();

        /** @brief Turn all channels on at full intensity. */
        void on();

    private:
        LED& redLed;
        LED& greenLed;
        LED& blueLed;
};

inline RgbLED::RgbLED(LED& red, LED& green, LED& blue) : redLed(red), greenLed(green), blueLed(blue) {}

inline void RgbLED::begin()
{
    redLed.begin();
    greenLed.begin();
    blueLed.begin();
}

inline void RgbLED::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    redLed.setIntensity(r);
    greenLed.setIntensity(g);
    blueLed.setIntensity(b);
}

inline void RgbLED::setColor(const RGB& color)
{
    setColor(color.r, color.g, color.b);
}

inline void RgbLED::off()
{
    setColor(0, 0, 0);
}

inline void RgbLED::on()
{
    setColor(255, 255, 255);
}
