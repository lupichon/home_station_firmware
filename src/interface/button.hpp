#pragma once

#include <Arduino.h>

class Button
{
    private:
        uint8_t pin;

        bool currentState;
        bool lastState;
        bool pressed;

        unsigned long lastDebounceTime;

        static constexpr unsigned long DEBOUNCE_TIME = 50; 

    public:
        Button(uint8_t pin);

        void begin();
        void update();

        bool wasPressed();
        bool isHeld() const;
};


inline Button::Button(uint8_t pin)
    : pin(pin),
      currentState(HIGH),
      lastState(HIGH),
      pressed(false),
      lastDebounceTime(0)
{
}


inline void Button::begin()
{
    pinMode(pin, INPUT_PULLUP);

    currentState = HIGH;
    lastState = HIGH;
    pressed = false;
}


inline void Button::update()
{
    bool reading = digitalRead(pin);

    if (reading != lastState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME)
    {
        if (reading != currentState)
        {
            currentState = reading;

            // Bouton pressé
            if (currentState == LOW)
            {
                pressed = true;
            }
        }
    }

    lastState = reading;
}


inline bool Button::wasPressed()
{
    if (pressed)
    {
        pressed = false;
        return true;
    }

    return false;
}

inline bool Button::isHeld() const
{
    return currentState == LOW;
}