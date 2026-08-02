#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/measurement.hpp"
#include "../core/gas_state.hpp"

class DisplaySSD1306
{
    public:
        enum class Page
        {
            TEMPERATURE,
            HUMIDITY,
            CO2,
            GASLEVEL,
            LUMINOSITY,
            MOTION, 
            SOUND, 
            OBSTACLE,
            VIBRATION
        };

        static constexpr uint8_t WIDTH = 128;
        static constexpr uint8_t HEIGHT = 64;
        static constexpr uint8_t I2C_ADDRESS = 0x3C;
        static constexpr unsigned long SLEEP_TIMEOUT_MS = 60000;

        DisplaySSD1306();

        bool begin();
        void update(const Measurement& measurement);
        void nextPage();
        bool isSleeping() const;
        void buttonPressed();

    private:
        Adafruit_SSD1306 screen;
        Page currentPage;
        bool sleeping;
        unsigned long lastActivityTime;

        void displayTemperature(float value);
        void displayHumidity(float value);
        void displayCO2(uint16_t value);
        void displayGasLevel(uint16_t value);
        void displayLuminosity(float value);
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


inline DisplaySSD1306::DisplaySSD1306() : screen(WIDTH, HEIGHT, &Wire, -1),
      currentPage(Page::TEMPERATURE), sleeping(false), lastActivityTime(0)
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
        case Page::TEMPERATURE:
            currentPage = Page::HUMIDITY;
            break;

        case Page::HUMIDITY:
            currentPage = Page::CO2;
            break;

        case Page::CO2:
            currentPage = Page::GASLEVEL;
            break;

        case Page::GASLEVEL:
            currentPage = Page::LUMINOSITY;
            break;

        case Page::LUMINOSITY:
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
            currentPage = Page::TEMPERATURE;
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

inline void DisplaySSD1306::buttonPressed()
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