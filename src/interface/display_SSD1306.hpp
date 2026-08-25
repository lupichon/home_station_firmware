/**
 * @file    display_ssd1306.hpp
 * @brief   SSD1306 OLED display interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-28
 */

#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/measurement.hpp"
#include "../core/gas_state.hpp"
#include "../core/clock.hpp"

// ============================================================
// DisplaySSD1306 class definition
// ============================================================

class DisplaySSD1306
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Pages available for cyclic display on the screen.
         */
        enum class Page
        {
            CLOCK,
            TEMPERATURE,
            HUMIDITY,
            CO2,
            GASLEVEL,
            LUMINOSITY,
            MOTION,
            SOUND,
            OBSTACLE,
            VIBRATION,
            PRESSURE,
            VOC,
            NOX
        };

        static constexpr uint8_t WIDTH = 128;                       // Screen width in pixels
        static constexpr uint8_t HEIGHT = 64;                       // Screen height in pixels
        static constexpr uint8_t I2C_ADDRESS = 0x3C;                // I2C address of the SSD1306 controller
        static constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;    // Idle delay before the screen goes to sleep

        /**
         * @brief Constructor for DisplaySSD1306.
         * @param clock Reference to the Clock instance used to render the clock page.
         */
        DisplaySSD1306(Clock &clock);

        /**
         * @brief Initialize the SSD1306 controller and show the startup screen.
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
        Adafruit_SSD1306 screen;         // Underlying SSD1306 driver instance
        Page currentPage;                // Page currently shown on screen
        bool sleeping;                   // Whether the screen is currently sleeping
        unsigned long lastActivityTime;  // Timestamp of the last user interaction
        Clock &clock;                    // Reference to the shared Clock instance

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
         * @param value Raw gas sensor reading, converted to a GasState label.
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
         * @brief Draw the common page header (title text and separator line).
         * @param title Title text to display at the top of the page.
         */
        void displayTitle(const char* title);

        /**
         * @brief Render a titled numeric value with its unit, or "N/A" if NaN.
         * @param title     Page title.
         * @param unit      Unit suffix printed after the value.
         * @param precision Number of decimal digits to display.
         * @param value     Numeric value to display.
         */
        void displayValue(String title, String unit, int precision, float value);

        /**
         * @brief Render a titled boolean state as one of two text labels.
         * @param title             Page title.
         * @param textIfDetected    Text shown when detected is true.
         * @param textIfNotDetected Text shown when detected is false.
         * @param detected          Boolean state to display.
         */
        void displayValue(String title, String textIfDetected, String textIfNotDetected, bool detected);

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
// implementations of DisplaySSD1306 methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline DisplaySSD1306::DisplaySSD1306(Clock &clock) : screen(WIDTH, HEIGHT, &Wire, -1),
      currentPage(Page::CLOCK), sleeping(false), lastActivityTime(0), clock(clock)
{
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool DisplaySSD1306::begin()
{
    if (!screen.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        return false;
    }

    screen.clearDisplay();

    screen.setTextColor(SSD1306_WHITE);

    screen.setTextSize(1);
    screen.setCursor(0, 0);
    screen.println("HomeStation");

    screen.display();

    lastActivityTime = millis();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────

inline void DisplaySSD1306::update(const Measurement& measurement)
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
    screen.clearDisplay();

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

        case Page::LUMINOSITY:
            displayLuminosity(measurement.luminosity);
            break;
            
        case Page::PRESSURE:
            displayPressure(measurement.pressure);
            break;

        case Page::VOC:
            displayVocIndex(measurement.vocIndex);
            break;

        case Page::NOX:
            displayNoxIndex(measurement.noxIndex);
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

        case Page::GASLEVEL:
            displayGasLevel(measurement.gasRaw);
            break;
    }

    screen.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// Page Navigation
// ─────────────────────────────────────────────────────────────────────────────

inline void DisplaySSD1306::nextPage()
{
    switch (currentPage)
    {
        case Page::CLOCK:
            currentPage = Page::TEMPERATURE;
            break;

        case Page::TEMPERATURE:
            currentPage = Page::HUMIDITY;
            break;

        case Page::HUMIDITY:
            currentPage = Page::CO2;
            break;

        case Page::CO2:
            currentPage = Page::VOC;
            break;

        case Page::VOC:
            currentPage = Page::NOX;
            break;

        case Page::NOX:
            currentPage = Page::GASLEVEL;
            break;

        case Page::GASLEVEL:
            currentPage = Page::LUMINOSITY;
            break;

        case Page::LUMINOSITY:
            currentPage = Page::PRESSURE;
            break;

        case Page::PRESSURE:
            currentPage = Page::MOTION;
            break;

        case Page::MOTION:
            currentPage = Page::SOUND;
            break;

        case Page::SOUND:
            currentPage = Page::OBSTACLE;
            break;

        case Page::OBSTACLE:
            currentPage = Page::VIBRATION;
            break;

        case Page::VIBRATION:
            currentPage = Page::CLOCK;
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Common Drawing Helpers
// ─────────────────────────────────────────────────────────────────────────────

inline void DisplaySSD1306::displayTitle(const char* title)
{
    screen.setTextSize(1);
    screen.setCursor(0, 0);
    screen.println(title);

    screen.drawLine(
        0,
        10,
        WIDTH - 1,
        10,
        SSD1306_WHITE
    );
}

inline void DisplaySSD1306::displayValue(String title, String unit, int precision, float value)
{
    displayTitle(title.c_str());

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (isnan(value))
    {
        screen.println("N/A");
    }
    else
    {
        screen.print(value, precision);
        screen.print(" ");
        screen.println(unit);
    }
}

inline void DisplaySSD1306::displayValue(String title, String textIfDetected, String textIfNotDetected, bool detected)
{
    displayTitle(title.c_str());

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (detected)
    {
        screen.println(textIfDetected);
    }
    else
    {
        screen.println(textIfNotDetected);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Pages
// ─────────────────────────────────────────────────────────────────────────────

inline void DisplaySSD1306::displayClock(uint32_t epoch)
{
    displayTitle("Clock");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (!clock.isSynchronized())
    {
        screen.println("N/A");
        return;
    }

    char time[9];
    snprintf(time, sizeof(time), "%02u:%02u:%02u",
        clock.hour(epoch), clock.minute(epoch), clock.second(epoch));
    screen.println(time);

    char date[11];
    snprintf(date, sizeof(date), "%02u/%02u/%04u",
        clock.day(epoch), clock.month(epoch), clock.year(epoch));
    screen.println(date);
}
 
inline void DisplaySSD1306::displayTemperature(float value)
{
    displayValue("Temperature", " C", 1, value);
}


inline void DisplaySSD1306::displayHumidity(float value)
{
    displayValue("Humidity", "%", 1, value);
}

inline void DisplaySSD1306::displayCO2(uint16_t value)
{
    displayValue("CO2", "ppm", 0, value);
}

inline void DisplaySSD1306::displayGasLevel(uint16_t raw)
{
    GasState state = gasStateFromRaw(raw);
    const char* stateStr = gasStateToString(state);

    displayValue("Gas Level", stateStr, 0, raw);
}

inline void DisplaySSD1306::displayLuminosity(float value)
{
    displayValue("Luminosity", "lux", 0, value);
}

inline void DisplaySSD1306::displayVocIndex(uint16_t value)
{
    displayValue("VOC", "", 0, value);
}

inline void DisplaySSD1306::displayNoxIndex(uint16_t value)
{
    displayValue("NOx", "", 0, value);
}

inline void DisplaySSD1306::displayMotion(bool detected)
{
    displayValue("Motion", "DETECTED", "NONE", detected);
}

inline void DisplaySSD1306::displaySound(bool detected)
{
    displayValue("Sound", "DETECTED", "QUIET", detected);
}

inline void DisplaySSD1306::displayObstacle(bool detected)
{
    displayValue("Obstacle", "DETECTED", "NONE", detected);
}

inline void DisplaySSD1306::displayVibration(bool detected)
{
    displayValue("Vibration", "DETECTED", "NONE", detected);
}

inline void DisplaySSD1306::displayPressure(float value)
{
    displayValue("Pressure", "hPa", 1, value);
}

// ─────────────────────────────────────────────────────────────────────────────
// Sleep Management
// ─────────────────────────────────────────────────────────────────────────────

inline bool DisplaySSD1306::isSleeping() const
{
    return sleeping; 
}

inline void DisplaySSD1306::sleep()
{
    screen.clearDisplay();
    screen.display();

    screen.ssd1306_command(SSD1306_DISPLAYOFF);

    sleeping = true;
}

inline void DisplaySSD1306::wake()
{
    screen.ssd1306_command(SSD1306_DISPLAYON);
    sleeping = false;
    lastActivityTime = millis();
}

inline void DisplaySSD1306::interact()
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