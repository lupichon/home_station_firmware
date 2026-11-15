/**
 * @file    main.cpp
 * @brief   Standalone tool to tune the status RGB LED color live over serial.
 *
 * Keys (serial monitor, 115200):
 *   r / g / b   select channel
 *   + / -       adjust selected channel by 1
 *   ] / [       adjust selected channel by 10
 *   0 / f       selected channel to 0 / 255
 *   n           next channel
 *   p           print the current color as an RGB constant
 */

#include <Arduino.h>
#include "core/pins.hpp"
#include "interface/led/rgb_led.hpp"

LED    ledRed(STATUS_LED_RED_PIN,     0);
LED    ledGreen(STATUS_LED_GREEN_PIN, 1);
LED    ledBlue(STATUS_LED_BLUE_PIN,   2);
RgbLED rgbLed(ledRed, ledGreen, ledBlue);

uint8_t     values[3] = {0, 0, 0};
uint8_t     selected  = 0;
const char  NAMES[3]  = {'R', 'G', 'B'};

void apply()
{
    rgbLed.setColor(values[0], values[1], values[2]);
    Serial.printf("[%c] R=%3u G=%3u B=%3u\n", NAMES[selected], values[0], values[1], values[2]);
}

void adjust(int delta)
{
    values[selected] = (uint8_t)constrain((int)values[selected] + delta, 0, 255);
    apply();
}

void setup()
{
    Serial.begin(115200);
    rgbLed.begin();
    Serial.println("LED tuner: r/g/b select, +/- (1), ]/[ (10), 0/f min/max, n next, p print");
    apply();
}

void loop()
{
    if (!Serial.available()) return;

    switch (Serial.read())
    {
        case 'r': selected = 0; apply(); break;
        case 'g': selected = 1; apply(); break;
        case 'b': selected = 2; apply(); break;
        case 'n': selected = (selected + 1) % 3; apply(); break;
        case '+': adjust(1);   break;
        case '-': adjust(-1);  break;
        case ']': adjust(10);  break;
        case '[': adjust(-10); break;
        case '0': values[selected] = 0;   apply(); break;
        case 'f': values[selected] = 255; apply(); break;
        case 'p':
            Serial.printf("static constexpr RGB NAME = {%3u, %3u, %3u};\n",
                          values[0], values[1], values[2]);
            break;
        default: break;
    }
}