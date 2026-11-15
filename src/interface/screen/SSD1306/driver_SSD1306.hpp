/**
 * @file    driver_SSD1306.hpp
 * @brief   SSD1306 OLED driver for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-28
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../screen_interface.hpp"

// ============================================================
// SSD1306 class definition
// ============================================================

class SSD1306 : public ScreenInterface
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for SSD1306.
         * @param wire I2C bus used to talk to the controller (defaults to Wire).
         */
        explicit SSD1306(TwoWire &wire = Wire);

        /**
         * @brief Initialize the SSD1306 controller and clear the screen.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Clear the frame buffer. Nothing is shown until flush() is called.
         */
        void clear() override;

        /**
         * @brief Push the frame buffer to the screen.
         */
        void flush() override;

        /**
         * @brief Set the text size used by the next print calls.
         * @param size Text size multiplier (1 = 6x8 pixels per character).
         */
        void setTextSize(uint8_t size) override;

        /**
         * @brief Move the text cursor.
         * @param x X-coordinate in pixels.
         * @param y Y-coordinate in pixels.
         */
        void setCursor(int16_t x, int16_t y) override;

        /**
         * @brief Print text at the current cursor position.
         * @param text Null-terminated string to print.
         */
        void print(const char* text) override;

        /**
         * @brief Print text at the current cursor position, then move to the next line.
         * @param text Null-terminated string to print.
         */
        void println(const char* text) override;

        /**
         * @brief Draw a horizontal line.
         * @param x      X-coordinate of the left end, in pixels.
         * @param y      Y-coordinate of the line, in pixels.
         * @param length Length of the line in pixels.
         */
        void drawHLine(int16_t x, int16_t y, int16_t length) override;

        /**
         * @brief Switch the panel on or off. The frame buffer is preserved.
         * @param on true to turn the panel on, false to turn it off.
         */
        void setPower(bool on) override;

        /**
         * @brief Get the screen width.
         * @return Screen width in pixels.
         */
        int16_t width() const override;

        /**
         * @brief Get the screen height.
         * @return Screen height in pixels.
         */
        int16_t height() const override;

    // ── Private members ───────────────────────────────────────────────────
    private:
        Adafruit_SSD1306 screen;         // Underlying SSD1306 driver instance

        static constexpr uint8_t WIDTH = 128;                       // Screen width in pixels
        static constexpr uint8_t HEIGHT = 64;                       // Screen height in pixels
        static constexpr uint8_t I2C_ADDRESS = 0x3C;                // I2C address of the SSD1306 controller
};


// ============================================================
// implementations of SSD1306 methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline SSD1306::SSD1306(TwoWire &wire) : screen(WIDTH, HEIGHT, &wire, -1)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool SSD1306::begin()
{
    if (!screen.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        return false;
    }

    screen.setTextColor(SSD1306_WHITE);
    screen.clearDisplay();
    screen.display();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Frame buffer
// ─────────────────────────────────────────────────────────────────────────────

inline void SSD1306::clear()
{
    screen.clearDisplay();
}

inline void SSD1306::flush()
{
    screen.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// Drawing primitives
// ─────────────────────────────────────────────────────────────────────────────

inline void SSD1306::setTextSize(uint8_t size)
{
    screen.setTextSize(size);
}

inline void SSD1306::setCursor(int16_t x, int16_t y)
{
    screen.setCursor(x, y);
}

inline void SSD1306::print(const char* text)
{
    screen.print(text);
}

inline void SSD1306::println(const char* text)
{
    screen.println(text);
}

inline void SSD1306::drawHLine(int16_t x, int16_t y, int16_t length)
{
    screen.drawFastHLine(x, y, length, SSD1306_WHITE);
}

// ─────────────────────────────────────────────────────────────────────────────
// Power
// ─────────────────────────────────────────────────────────────────────────────

inline void SSD1306::setPower(bool on)
{
    screen.ssd1306_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
}

// ─────────────────────────────────────────────────────────────────────────────
// Dimensions
// ─────────────────────────────────────────────────────────────────────────────

inline int16_t SSD1306::width() const
{
    return WIDTH;
}

inline int16_t SSD1306::height() const
{
    return HEIGHT;
}