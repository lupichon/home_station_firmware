#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/measurement.hpp"

class DisplaySSD1306
{
    public:
        enum class Page
        {
            TEMPERATURE,
            HUMIDITY,
            CO2,
            LUMINOSITY,
            MOTION, 
            SOUND, 
            OBSTACLE
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
        void displayCO2(float value);
        void displayLuminosity(float value);
        void displayMotion(bool detected);
        void displaySound(bool detected);
        void displayObstacle(bool detected);
        void displayTitle(const char* title);

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


inline void DisplaySSD1306::displayTemperature(float value)
{
    displayTitle("Temperature");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (isnan(value))
    {
        screen.println("N/A");
    }
    else
    {
        screen.print(value, 1);
        screen.println(" C");
    }
}


inline void DisplaySSD1306::displayHumidity(float value)
{
    displayTitle("Humidity");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (isnan(value))
    {
        screen.println("N/A");
    }
    else
    {
        screen.print(value, 1);
        screen.println(" %");
    }
}

inline void DisplaySSD1306::displayCO2(float value)
{
    displayTitle("CO2");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (isnan(value))
    {
        screen.println("N/A");
    }
    else
    {
        screen.print(value, 0);
        screen.println(" ppm");
    }
}


inline void DisplaySSD1306::displayLuminosity(float value)
{
    displayTitle("Luminosity");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (isnan(value))
    {
        screen.println("N/A");
    }
    else
    {
        screen.print(value, 1);
        screen.println(" lx");
    }
}


inline void DisplaySSD1306::displayMotion(bool detected)
{
    displayTitle("Motion");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (detected)
    {
        screen.println("DETECTED");
    }
    else
    {
        screen.println("NONE");
    }
}

inline void DisplaySSD1306::displaySound(bool detected)
{
    displayTitle("Sound");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (detected)
    {
        screen.println("DETECTED");
    }
    else
    {
        screen.println("NONE");
    }
}

inline void DisplaySSD1306::displayObstacle(bool detected)
{
    displayTitle("Obstacle");

    screen.setTextSize(2);
    screen.setCursor(0, 25);

    if (detected)
    {
        screen.println("DETECTED");
    }
    else
    {
        screen.println("NONE");
    }
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