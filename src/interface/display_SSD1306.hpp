#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/measurement.hpp"
#include "../core/gas_state.hpp"
#include "../core/clock.hpp"

class DisplaySSD1306
{
    public:
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

        static constexpr uint8_t WIDTH = 128;
        static constexpr uint8_t HEIGHT = 64;
        static constexpr uint8_t I2C_ADDRESS = 0x3C;
        static constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;

        DisplaySSD1306(Clock &clock);

        bool begin();
        void update(const Measurement& measurement);
        void nextPage();
        bool isSleeping() const;
        void interact();

    private:
        Adafruit_SSD1306 screen;
        Page currentPage;
        bool sleeping;
        unsigned long lastActivityTime;
        Clock &clock;

        void displayClock(uint32_t epoch);
        void displayTemperature(float value);
        void displayHumidity(float value);
        void displayCO2(uint16_t value);
        void displayVocIndex(uint16_t value);
        void displayNoxIndex(uint16_t value);
        void displayGasLevel(uint16_t value);
        void displayLuminosity(float value);
        void displayPressure(float value);
        void displayMotion(bool detected);
        void displaySound(bool detected);
        void displayObstacle(bool detected);
        void displayVibration(bool detected);
        void displayTitle(const char* title);
        void displayValue(String title, String unit, int precision, float value);
        void displayValue(String title, String textIfDetected, String textIfNotDetected, bool detected);

        void sleep();
        void wake();
};


inline DisplaySSD1306::DisplaySSD1306(Clock &clock) : screen(WIDTH, HEIGHT, &Wire, -1),
      currentPage(Page::CLOCK), sleeping(false), lastActivityTime(0), clock(clock)
{
}


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