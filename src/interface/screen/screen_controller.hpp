/**
 * @file    screen_controller.hpp
 * @brief   Display logic for the HomeStation device: pages, navigation,
 *          inactivity sleep and value formatting. The screen itself is
 *          driven through ScreenInterface.
 * @author  Lucas Pichon
 * @date    2026-07-28
 */

#pragma once

#include <Arduino.h>
#include <math.h>
#include <stdio.h>

#include "screen_interface.hpp"
#include "../../core/measurement.hpp"
#include "../../core/clock.hpp"
#include "../../core/firmware_version.hpp"

// ============================================================
// ScreenController class definition
// ============================================================

class ScreenController
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Pages available for cyclic display on the screen.
         *        Declared in display order: nextPage() simply goes to the
         *        following value, and COUNT must stay last.
         */
        enum class Page : uint8_t
        {
            CLOCK,
            TEMPERATURE,
            HUMIDITY,
            CO2,
            VOC,
            NOX,
            GASLEVEL,
            LUMINOSITY,
            PRESSURE,
            MOTION,
            SOUND,
            OBSTACLE,
            VIBRATION,
            VERSION,
            COUNT
        };

        /**
         * @brief Constructor for ScreenController.
         * @param driver Reference to the screen driver used for all drawing.
         * @param clock  Reference to the Clock instance used to render the clock page.
         */
        ScreenController(ScreenInterface &driver, Clock &clock);

        /**
         * @brief Initialize the screen driver and show the startup screen.
         * @return true if initialization was successful, false otherwise.
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
        Clock &clock;                    // Reference to the shared Clock instance
        Page currentPage;                // Page currently shown on screen
        bool sleeping;                   // Whether the screen is currently sleeping
        unsigned long lastActivityTime;  // Timestamp of the last user interaction

        static constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;    // Idle delay before the screen goes to sleep

        static constexpr uint8_t VALUE_TEXT_SIZE = 2;               // Text size for the measurement values
        static constexpr int16_t VALUE_CURSOR_X = 0;                // X-coordinate for the measurement value cursor
        static constexpr int16_t VALUE_CURSOR_Y = 25;               // Y-coordinate for the measurement value cursor
        static constexpr uint8_t TITLE_TEXT_SIZE = 1;               // Text size for the title
        static constexpr int16_t TITLE_CURSOR_X = 0;                // X-coordinate for the title cursor
        static constexpr int16_t TITLE_CURSOR_Y = 0;                // Y-coordinate for the title cursor
        static constexpr int16_t TITLE_SEPARATOR_Y = 10;            // Y-coordinate for the title separator line

        /**
         * @brief Render the CLOCK page (current time and date).
         * @param epoch Current epoch time.
         */
        void displayClock(uint32_t epoch);

        /**
         * @brief Render the TEMPERATURE page.
         * @param value Temperature value in degrees Celsius.
         */
        void displayTemperature(float value);

        /**
         * @brief Render the HUMIDITY page.
         * @param value Relative humidity value in percent.
         */
        void displayHumidity(float value);

        /**
         * @brief Render the CO2 page.
         * @param value CO2 concentration in ppm.
         */
        void displayCO2(uint16_t value);

        /**
         * @brief Render the VOC page.
         * @param value VOC index value.
         */
        void displayVocIndex(uint16_t value);

        /**
         * @brief Render the NOX page.
         * @param value NOx index value.
         */
        void displayNoxIndex(uint16_t value);

        /**
         * @brief Render the GASLEVEL page.
         * @param value Raw gas sensor reading.
         */
        void displayGasLevel(uint16_t value);

        /**
         * @brief Render the LUMINOSITY page.
         * @param value Luminosity value in lux.
         */
        void displayLuminosity(float value);

        /**
         * @brief Render the PRESSURE page.
         * @param value Atmospheric pressure value in hPa.
         */
        void displayPressure(float value);

        /**
         * @brief Render the MOTION page.
         * @param detected true if motion is currently detected, false otherwise.
         */
        void displayMotion(bool detected);

        /**
         * @brief Render the SOUND page.
         * @param detected true if sound is currently detected, false otherwise.
         */
        void displaySound(bool detected);

        /**
         * @brief Render the OBSTACLE page.
         * @param detected true if an obstacle is currently detected, false otherwise.
         */
        void displayObstacle(bool detected);

        /**
         * @brief Render the VIBRATION page.
         * @param detected true if vibration is currently detected, false otherwise.
         */
        void displayVibration(bool detected);

        /**
         * @brief Render the VERSION page.
         * @param version Firmware version string.
         */
        void displayVersion(const char* version);

        /**
         * @brief Draw the common page header (title text and separator line).
         * @param title Title text to display at the top of the page.
         */
        void displayTitle(const char* title);

        /**
         * @brief Render a titled numeric value with its unit, or "N/A" if NaN.
         * @param title     Page title.
         * @param unit      Unit suffix printed after the value (may be empty).
         * @param precision Number of decimal digits to display.
         * @param value     Numeric value to display.
         */
        void displayNumber(const char* title, const char* unit, int precision, float value);

        /**
         * @brief Render a titled boolean state as one of two text labels.
         * @param title             Page title.
         * @param textIfDetected    Text shown when detected is true.
         * @param textIfNotDetected Text shown when detected is false.
         * @param detected          Boolean state to display.
         */
        void displayState(const char* title, const char* textIfDetected, const char* textIfNotDetected, bool detected);

        /**
         * @brief Render a titled text value.
         * @param title Page title.
         * @param text  Text to display.
         */
        void displayText(const char* title, const char* text);

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

inline ScreenController::ScreenController(ScreenInterface &driver, Clock &clock) : driver(driver), clock(clock),
      currentPage(Page::CLOCK), sleeping(false), lastActivityTime(0)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool ScreenController::begin()
{
    if (!driver.begin())
    {
        return false;
    }

    driver.clear();

    driver.setTextSize(TITLE_TEXT_SIZE);
    driver.setCursor(TITLE_CURSOR_X, TITLE_CURSOR_Y);
    driver.println("HomeStation");

    driver.flush();

    lastActivityTime = millis();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::update(const Measurement& measurement)
{
    if (sleeping)
    {
        return;
    }

    if (millis() - lastActivityTime >= SLEEP_TIMEOUT_MS)
    {
        sleep();
        return;
    }

    driver.clear();

    switch (currentPage)
    {
        case Page::CLOCK:
            displayClock(measurement.timestamp);
            break;

        case Page::TEMPERATURE:
            displayTemperature(measurement.temperature);
            break;

        case Page::HUMIDITY:
            displayHumidity(measurement.humidity);
            break;

        case Page::CO2:
            displayCO2(measurement.co2);
            break;

        case Page::VOC:
            displayVocIndex(measurement.vocIndex);
            break;

        case Page::NOX:
            displayNoxIndex(measurement.noxIndex);
            break;

        case Page::GASLEVEL:
            displayGasLevel(measurement.gasRaw);
            break;

        case Page::LUMINOSITY:
            displayLuminosity(measurement.luminosity);
            break;

        case Page::PRESSURE:
            displayPressure(measurement.pressure);
            break;

        case Page::MOTION:
            displayMotion(measurement.motion);
            break;

        case Page::SOUND:
            displaySound(measurement.sound);
            break;

        case Page::OBSTACLE:
            displayObstacle(measurement.obstacle);
            break;

        case Page::VIBRATION:
            displayVibration(measurement.vibration);
            break;

        case Page::VERSION:
            displayVersion(FIRMWARE_VERSION_STRING);
            break;

        default:
            break;
    }

    driver.flush();
}

// ─────────────────────────────────────────────────────────────────────────────
// Page Navigation
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::nextPage()
{
    currentPage = static_cast<Page>(
        (static_cast<uint8_t>(currentPage) + 1) % static_cast<uint8_t>(Page::COUNT));
}

// ─────────────────────────────────────────────────────────────────────────────
// Common Drawing Helpers
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::displayTitle(const char* title)
{
    driver.setTextSize(TITLE_TEXT_SIZE);
    driver.setCursor(TITLE_CURSOR_X, TITLE_CURSOR_Y);
    driver.println(title);

    driver.drawHLine(
        TITLE_CURSOR_X,
        TITLE_SEPARATOR_Y,
        driver.width() - 1
    );
}

inline void ScreenController::displayNumber(const char* title, const char* unit, int precision, float value)
{
    char buffer[24];

    if (isnan(value))
    {
        snprintf(buffer, sizeof(buffer), "N/A");
    }
    else if (unit[0] == '\0')
    {
        snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%.*f %s", precision, value, unit);
    }

    displayText(title, buffer);
}

inline void ScreenController::displayState(const char* title, const char* textIfDetected, const char* textIfNotDetected, bool detected)
{
    displayText(title, detected ? textIfDetected : textIfNotDetected);
}

inline void ScreenController::displayText(const char* title, const char* text)
{
    displayTitle(title);

    driver.setTextSize(VALUE_TEXT_SIZE);
    driver.setCursor(VALUE_CURSOR_X, VALUE_CURSOR_Y);

    driver.println(text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Pages
// ─────────────────────────────────────────────────────────────────────────────

inline void ScreenController::displayClock(uint32_t epoch)
{
    displayTitle("Clock");

    driver.setTextSize(VALUE_TEXT_SIZE);
    driver.setCursor(VALUE_CURSOR_X, VALUE_CURSOR_Y);

    if (!clock.isSynchronized())
    {
        driver.println("N/A");
        return;
    }

    char time[9];
    snprintf(time, sizeof(time), "%02u:%02u:%02u",
        clock.hour(epoch), clock.minute(epoch), clock.second(epoch));
    driver.println(time);

    char date[11];
    snprintf(date, sizeof(date), "%02u/%02u/%04u",
        clock.day(epoch), clock.month(epoch), clock.year(epoch));
    driver.println(date);
}

inline void ScreenController::displayTemperature(float value)
{
    displayNumber("Temperature", "C", 1, value);
}

inline void ScreenController::displayHumidity(float value)
{
    displayNumber("Humidity", "%", 1, value);
}

inline void ScreenController::displayCO2(uint16_t value)
{
    displayNumber("CO2", "ppm", 0, value);
}

inline void ScreenController::displayVocIndex(uint16_t value)
{
    displayNumber("VOC", "", 0, value);
}

inline void ScreenController::displayNoxIndex(uint16_t value)
{
    displayNumber("NOx", "", 0, value);
}

inline void ScreenController::displayGasLevel(uint16_t value)
{
    displayNumber("Gas raw", "", 0, value);
}

inline void ScreenController::displayLuminosity(float value)
{
    displayNumber("Luminosity", "lux", 0, value);
}

inline void ScreenController::displayPressure(float value)
{
    displayNumber("Pressure", "hPa", 1, value);
}

inline void ScreenController::displayMotion(bool detected)
{
    displayState("Motion", "DETECTED", "NONE", detected);
}

inline void ScreenController::displaySound(bool detected)
{
    displayState("Sound", "DETECTED", "QUIET", detected);
}

inline void ScreenController::displayObstacle(bool detected)
{
    displayState("Obstacle", "DETECTED", "NONE", detected);
}

inline void ScreenController::displayVibration(bool detected)
{
    displayState("Vibration", "DETECTED", "NONE", detected);
}

inline void ScreenController::displayVersion(const char* version)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "v%s", version);

    displayText("Version", buffer);
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