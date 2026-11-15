/**
 * @file    screen_layout.hpp
 * @brief   Page selection and rendering interface for screen layouts.
 * @author  Lucas Pichon
 * @date    2026-11-12
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "../screen_interface.hpp"
#include "../../../core/measurement.hpp"
#include "../../../core/clock.hpp"

/**
 * @brief Draws the startup screen and pages into a screen driver's buffer.
 *
 * The controller clears and presents the buffer around each rendering call.
 * Each layout defines its own pages and their order. The page count must be
 * nonzero and remain constant while the controller is using the layout.
 */
class ScreenLayout
{
    public:
        /// Destructor for the screen layout interface
        virtual ~ScreenLayout() = default;
        /// Return the number of pages in the layout
        virtual uint8_t pageCount() const = 0;
        /// Render the startup screen
        virtual void renderStartup(ScreenInterface& driver) = 0;
        /// Render the page at the given index
        virtual void renderPage(ScreenInterface& driver, uint8_t pageIndex, const Measurement& measurement, const Clock& clock) = 0;
};
