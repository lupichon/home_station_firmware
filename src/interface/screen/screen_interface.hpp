/**
 * @file    screen_interface.hpp
 * @brief   Interface for screen drivers used by the HomeStation display controller.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Drawing and power operations required by the screen controller.
 *
 * Drawing modifies a frame buffer; flush() presents it on the screen.
 * The controller currently lays out its pages for a 128x64 pixel display.
 */
class ScreenInterface
{
    public:
        virtual ~ScreenInterface() = default;

        /// Initialize the screen and return false if initialization fails.
        virtual bool begin() = 0;
        /// Clear the frame buffer without presenting it.
        virtual void clear() = 0;
        /// Present the frame buffer on the screen.
        virtual void flush() = 0;
        /// Set the text size multiplier used by subsequent text operations.
        virtual void setTextSize(uint8_t size) = 0;
        /// Set the text cursor position in pixels.
        virtual void setCursor(int16_t x, int16_t y) = 0;
        /// Write a null-terminated string at the current cursor position.
        virtual void print(const char* text) = 0;
        /// Write a null-terminated string and move to the next line.
        virtual void println(const char* text) = 0;
        /// Draw a horizontal line using pixel coordinates and length.
        virtual void drawHLine(int16_t x, int16_t y, int16_t length) = 0;
        /// Turn the screen on or off while preserving the frame buffer.
        virtual void setPower(bool on) = 0;
        /// Return the screen width in pixels.
        virtual int16_t width() const = 0;
        /// Return the screen height in pixels.
        virtual int16_t height() const = 0;
};
