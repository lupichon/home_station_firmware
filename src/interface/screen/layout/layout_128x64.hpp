/**
 * @file    layout_128x64.hpp
 * @brief   HomeStation page layout for a 128x64 pixel screen.
 * @author  Lucas Pichon
 * @date    2026-11-12
 */

#pragma once

#include <math.h>
#include <stdio.h>

#include "screen_layout.hpp"
#include "../../../core/firmware_version.hpp"

class Layout128x64 : public ScreenLayout
{
    public:
        uint8_t pageCount() const override;
        void renderStartup(ScreenInterface& driver) override;
        void renderPage(ScreenInterface& driver, uint8_t pageIndex,
                        const Measurement& measurement, const Clock& clock) override;

    private:
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
        void displayClock(ScreenInterface& driver, const Clock& clock, uint32_t epoch);

        /**
         * @brief Render the TEMPERATURE page.
         * @param value Temperature value in degrees Celsius.
         */
        void displayTemperature(ScreenInterface& driver, float value);

        /**
         * @brief Render the HUMIDITY page.
         * @param value Relative humidity value in percent.
         */
        void displayHumidity(ScreenInterface& driver, float value);

        /**
         * @brief Render the CO2 page.
         * @param value CO2 concentration in ppm.
         */
        void displayCO2(ScreenInterface& driver, uint16_t value);

        /**
         * @brief Render the VOC page.
         * @param value VOC index value.
         */
        void displayVocIndex(ScreenInterface& driver, uint16_t value);

        /**
         * @brief Render the NOX page.
         * @param value NOx index value.
         */
        void displayNoxIndex(ScreenInterface& driver, uint16_t value);

        /**
         * @brief Render the GASLEVEL page.
         * @param value Raw gas sensor reading.
         */
        void displayGasLevel(ScreenInterface& driver, uint16_t value);

        /**
         * @brief Render the LUMINOSITY page.
         * @param value Luminosity value in lux.
         */
        void displayLuminosity(ScreenInterface& driver, float value);

        /**
         * @brief Render the PRESSURE page.
         * @param value Atmospheric pressure value in hPa.
         */
        void displayPressure(ScreenInterface& driver, float value);

        /**
         * @brief Render the MOTION page.
         * @param detected true if motion is currently detected, false otherwise.
         */
        void displayMotion(ScreenInterface& driver, bool detected);

        /**
         * @brief Render the SOUND page.
         * @param detected true if sound is currently detected, false otherwise.
         */
        void displaySound(ScreenInterface& driver, bool detected);

        /**
         * @brief Render the OBSTACLE page.
         * @param detected true if an obstacle is currently detected, false otherwise.
         */
        void displayObstacle(ScreenInterface& driver, bool detected);

        /**
         * @brief Render the VIBRATION page.
         * @param detected true if vibration is currently detected, false otherwise.
         */
        void displayVibration(ScreenInterface& driver, bool detected);

        /**
         * @brief Render the VERSION page.
         * @param version Firmware version string.
         */
        void displayVersion(ScreenInterface& driver, const char* version);

        /**
         * @brief Draw the common page header (title text and separator line).
         * @param title Title text to display at the top of the page.
         */
        void displayTitle(ScreenInterface& driver, const char* title);

        /**
         * @brief Render a titled numeric value with its unit, or "N/A" if NaN.
         * @param title     Page title.
         * @param unit      Unit suffix printed after the value (may be empty).
         * @param precision Number of decimal digits to display.
         * @param value     Numeric value to display.
         */
        void displayNumber(ScreenInterface& driver, const char* title, const char* unit, int precision, float value);

        /**
         * @brief Render a titled boolean state as one of two text labels.
         * @param title             Page title.
         * @param textIfDetected    Text shown when detected is true.
         * @param textIfNotDetected Text shown when detected is false.
         * @param detected          Boolean state to display.
         */
        void displayState(ScreenInterface& driver, const char* title, const char* textIfDetected, const char* textIfNotDetected, bool detected);

        /**
         * @brief Render a titled text value.
         * @param title Page title.
         * @param text  Text to display.
         */
        void displayText(ScreenInterface& driver, const char* title, const char* text);

};

inline uint8_t Layout128x64::pageCount() const
{
    return static_cast<uint8_t>(Page::COUNT);
}

inline void Layout128x64::renderStartup(ScreenInterface& driver)
{
    driver.setTextSize(TITLE_TEXT_SIZE);
    driver.setCursor(TITLE_CURSOR_X, TITLE_CURSOR_Y);
    driver.println("HomeStation");
}

inline void Layout128x64::renderPage(ScreenInterface& driver, uint8_t pageIndex,
                                    const Measurement& measurement, const Clock& clock)
{
    switch (static_cast<Page>(pageIndex))
    {
        case Page::CLOCK:
            displayClock(driver, clock, measurement.timestamp);
            break;

        case Page::TEMPERATURE:
            displayTemperature(driver, measurement.temperature);
            break;

        case Page::HUMIDITY:
            displayHumidity(driver, measurement.humidity);
            break;

        case Page::CO2:
            displayCO2(driver, measurement.co2);
            break;

        case Page::VOC:
            displayVocIndex(driver, measurement.vocIndex);
            break;

        case Page::NOX:
            displayNoxIndex(driver, measurement.noxIndex);
            break;

        case Page::GASLEVEL:
            displayGasLevel(driver, measurement.gasRaw);
            break;

        case Page::LUMINOSITY:
            displayLuminosity(driver, measurement.luminosity);
            break;

        case Page::PRESSURE:
            displayPressure(driver, measurement.pressure);
            break;

        case Page::MOTION:
            displayMotion(driver, measurement.motion);
            break;

        case Page::SOUND:
            displaySound(driver, measurement.sound);
            break;

        case Page::OBSTACLE:
            displayObstacle(driver, measurement.obstacle);
            break;

        case Page::VIBRATION:
            displayVibration(driver, measurement.vibration);
            break;

        case Page::VERSION:
            displayVersion(driver, FIRMWARE_VERSION_STRING);
            break;

        default:
            break;
    }
}

inline void Layout128x64::displayTitle(ScreenInterface& driver, const char* title)
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

inline void Layout128x64::displayNumber(ScreenInterface& driver, const char* title, const char* unit, int precision, float value)
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

    displayText(driver, title, buffer);
}

inline void Layout128x64::displayState(ScreenInterface& driver, const char* title, const char* textIfDetected, const char* textIfNotDetected, bool detected)
{
    displayText(driver, title, detected ? textIfDetected : textIfNotDetected);
}

inline void Layout128x64::displayText(ScreenInterface& driver, const char* title, const char* text)
{
    displayTitle(driver, title);

    driver.setTextSize(VALUE_TEXT_SIZE);
    driver.setCursor(VALUE_CURSOR_X, VALUE_CURSOR_Y);

    driver.println(text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Pages
// ─────────────────────────────────────────────────────────────────────────────

inline void Layout128x64::displayClock(ScreenInterface& driver, const Clock& clock, uint32_t epoch)
{
    displayTitle(driver, "Clock");

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

inline void Layout128x64::displayTemperature(ScreenInterface& driver, float value)
{
    displayNumber(driver, "Temperature", "C", 1, value);
}

inline void Layout128x64::displayHumidity(ScreenInterface& driver, float value)
{
    displayNumber(driver, "Humidity", "%", 1, value);
}

inline void Layout128x64::displayCO2(ScreenInterface& driver, uint16_t value)
{
    displayNumber(driver, "CO2", "ppm", 0, value);
}

inline void Layout128x64::displayVocIndex(ScreenInterface& driver, uint16_t value)
{
    displayNumber(driver, "VOC", "", 0, value);
}

inline void Layout128x64::displayNoxIndex(ScreenInterface& driver, uint16_t value)
{
    displayNumber(driver, "NOx", "", 0, value);
}

inline void Layout128x64::displayGasLevel(ScreenInterface& driver, uint16_t value)
{
    displayNumber(driver, "Gas raw", "", 0, value);
}

inline void Layout128x64::displayLuminosity(ScreenInterface& driver, float value)
{
    displayNumber(driver, "Luminosity", "lux", 0, value);
}

inline void Layout128x64::displayPressure(ScreenInterface& driver, float value)
{
    displayNumber(driver, "Pressure", "hPa", 1, value);
}

inline void Layout128x64::displayMotion(ScreenInterface& driver, bool detected)
{
    displayState(driver, "Motion", "DETECTED", "NONE", detected);
}

inline void Layout128x64::displaySound(ScreenInterface& driver, bool detected)
{
    displayState(driver, "Sound", "DETECTED", "QUIET", detected);
}

inline void Layout128x64::displayObstacle(ScreenInterface& driver, bool detected)
{
    displayState(driver, "Obstacle", "DETECTED", "NONE", detected);
}

inline void Layout128x64::displayVibration(ScreenInterface& driver, bool detected)
{
    displayState(driver, "Vibration", "DETECTED", "NONE", detected);
}

inline void Layout128x64::displayVersion(ScreenInterface& driver, const char* version)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "v%s", version);

    displayText(driver, "Version", buffer);
}
