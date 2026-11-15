/**
 * @file    screen_controller.hpp
 * @brief   Display navigation and inactivity sleep for the HomeStation device.
 *          Page rendering is delegated to a screen layout.
 * @author  Lucas Pichon
 * @date    2026-07-28
 */

#pragma once

#include <Arduino.h>

#include "layout/screen_layout.hpp"
#include "../../core/measurement.hpp"
#include "../../core/clock.hpp"

// ============================================================
// ScreenController class definition
// ============================================================

class ScreenController
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for ScreenController.
         * @param driver Reference to the screen driver used for all drawing.
         * @param layout Reference to the layout used to render pages.
         * @param clock  Reference to the Clock instance used to render the clock page.
         */
        ScreenController(ScreenInterface &driver, ScreenLayout &layout, Clock &clock);

        /**
         * @brief Initialize the screen driver and show the startup screen.
         * @return true if initialization was successful, false if the layout
         *         has no pages or the screen driver failed to initialize.
         */
        bool begin();

        /**
         * @brief Render the current page with the given measurement values.
         *        Automatically puts the screen to sleep after SLEEP_TIMEOUT_MS
         *        of inactivity. Does nothing while the screen is sleeping.
         * @param measurement Latest set of sensor measurements to display.
         */
        void update(const Measurement& measurement);

        /**
         * @brief Advance to the next page in the display cycle.
         */
        void nextPage();

        /**
         * @brief Check whether the screen is currently in sleep mode.
         * @return true if the screen is sleeping, false otherwise.
         */
        bool isSleeping() const;

        /**
         * @brief Handle a user interaction with the display.
         *        Wakes the screen if sleeping, otherwise advances to the next
         *        page and resets the inactivity timer.
         */
        void interact();

    // ── Private members ───────────────────────────────────────────────────
    private:
        ScreenInterface &driver;         // Reference to the screen driver
        ScreenLayout &layout;            // Reference to the page layout
        Clock &clock;                    // Reference to the shared Clock instance
        uint8_t currentPage;             // Index of the page currently shown
        bool initialized;                // Whether the screen has been initialized
        bool sleeping;                   // Whether the screen is currently sleeping
        unsigned long lastActivityTime;  // Timestamp of the last user interaction

        static constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;    // Idle delay before the screen goes to sleep

        /**
         * @brief Turn the screen off and mark it as sleeping.
         */
        void sleep();

        /**
         * @brief Turn the screen back on and reset the inactivity timer.
         */
        void wake();
};


// ============================================================
// implementations of ScreenController methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline ScreenController::ScreenController(ScreenInterface &driver, ScreenLayout &layout, Clock &clock)
    : driver(driver), layout(layout), clock(clock),
      currentPage(0), initialized(false), sleeping(false), lastActivityTime(0)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool ScreenController::begin()
{
    if (!driver.begin() || layout.pageCount() == 0)
    {
        initialized = false;
        return false;
    }

    initialized = true;

    driver.clear();

    layout.renderStartup(driver);

    driver.flush();

    lastActivityTime = millis();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::update(const Measurement& measurement)
{
    if (!initialized || sleeping)
    {
        return;
    }

    if (millis() - lastActivityTime >= SLEEP_TIMEOUT_MS)
    {
        sleep();
        return;
    }

    driver.clear();

    layout.renderPage(driver, currentPage, measurement, clock);

    driver.flush();
}

// ─────────────────────────────────────────────────────────────────────────────
// Page Navigation
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::nextPage()
{
    if (currentPage + 1 < layout.pageCount())
    {
        currentPage++;
    }
    else
    {
        currentPage = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Sleep Management
// ─────────────────────────────────────────────────────────────────────────────

inline bool ScreenController::isSleeping() const
{
    return sleeping;
}

inline void ScreenController::sleep()
{
    driver.clear();
    driver.flush();

    driver.setPower(false);

    sleeping = true;
}

inline void ScreenController::wake()
{
    driver.setPower(true);

    sleeping = false;
    lastActivityTime = millis();
}

inline void ScreenController::interact()
{
    if (!initialized)
    {
        return;
    }
    
    if (sleeping)
    {
        wake();
    }
    else
    {
        lastActivityTime = millis();
        nextPage();
    }
}