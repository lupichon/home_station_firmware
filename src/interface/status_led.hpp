#pragma once
#include <Arduino.h>

class StatusLED
{
    public:
        enum class State
        {
            IDLE,
            STARTING,
            OK,
            WARNING_SENSOR,
            WARNING_COMMUNICATION,
            ERROR
        };

        StatusLED(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
        void begin();
        void setState(State state);
        void update();          
        void setColor(uint8_t red, uint8_t green, uint8_t blue);

    private:
        uint8_t redPin, greenPin, bluePin;
        State   state;

        // Pour le clignotement
        unsigned long lastToggleMs = 0;
        bool          ledOn        = false;

        struct BlinkProfile {
            uint8_t  r, g, b;       // Couleur
            uint16_t onMs, offMs;   // Durées ON / OFF
            bool     blink;         // false = couleur fixe
        };

        static BlinkProfile profileFor(State s);
        void applyProfile(const BlinkProfile& p);
    };

// ─── Implémentation ───────────────────────────────────────────────────────────

StatusLED::StatusLED(uint8_t r, uint8_t g, uint8_t b)
    : redPin(r), greenPin(g), bluePin(b), state(State::IDLE) {}

void StatusLED::begin()
{
    pinMode(redPin,   OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin,  OUTPUT);
    setState(State::IDLE);
}

void StatusLED::setState(State newState)
{
    if (newState == state) return; // Pas de changement
    state       = newState;
    lastToggleMs = millis();
    ledOn       = true;          // On commence toujours par la phase ON
    applyProfile(profileFor(state));
}

// Appelé dans loop() — ne bloque jamais
void StatusLED::update()
{
    BlinkProfile p = profileFor(state);
    if (!p.blink) return;        // Couleur fixe : rien à faire

    unsigned long now     = millis();
    unsigned long elapsed = now - lastToggleMs;
    uint16_t      target  = ledOn ? p.onMs : p.offMs;

    if (elapsed >= target)
    {
        lastToggleMs = now;
        ledOn = !ledOn;
        applyProfile(p);
    }
}

void StatusLED::applyProfile(const BlinkProfile& p)
{
    if (!p.blink || ledOn)
    {
        setColor(p.r, p.g, p.b);
    }
    else
    {
        setColor(0, 0, 0);
    }
}

void StatusLED::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(redPin,   r);
    analogWrite(greenPin, g);
    analogWrite(bluePin,  b);
}

// ─── Profils par état ─────────────────────────────────────────────────────────
//                              R    G    B    onMs  offMs  blink
StatusLED::BlinkProfile StatusLED::profileFor(State s)
{
    switch (s)
    {
        case State::IDLE:                   return {  0,   0,   0,    0,    0,   false};
        case State::STARTING:               return {  0,   0, 255,    0,    0,   false};  // bleu fixe
        case State::OK:                     return {  0, 255,   0,  100,  5000,  true}; // vert  clignotant très brièvement
        case State::WARNING_SENSOR:         return {255, 75,   0,  500,    500,  true};  // jaune clignotant lent
        case State::WARNING_COMMUNICATION:  return {128,   0, 255,  500,    500, true};  // violet clignotant lent
        case State::ERROR:                  return {255,   0,   0,  100,    100, true};  // rouge clignotant très rapide
        default:                            return {  0,   0,   0,    0,    0,   false};
    }
}