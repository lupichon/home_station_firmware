#pragma once
#include <Arduino.h>

struct RGB
{
    uint8_t r, g, b;
};

namespace Delay
{
    static constexpr unsigned long MS_0    = 0;
    static constexpr unsigned long MS_10   = 10;
    static constexpr unsigned long MS_100  = 100;
    static constexpr unsigned long MS_500  = 500;
    static constexpr unsigned long MS_1000 = 1000;
    static constexpr unsigned long MS_5000 = 5000;
}

namespace LedColor
{
    static constexpr RGB OFF     = {  0,   0,   0};
    static constexpr RGB BLUE    = {  0,   0, 255};
    static constexpr RGB MAGENTA = {255,   0, 255};
    static constexpr RGB GREEN   = {  0, 255,   0};
    static constexpr RGB ORANGE  = {255,  75,   0};
    static constexpr RGB PURPLE  = {128,   0, 255};
    static constexpr RGB RED     = {255,   0,   0};
    static constexpr RGB CYAN    = {  0, 255, 255};
    static constexpr RGB YELLOW  = {255, 255,   0};
    static constexpr RGB WHITE   = {255, 255, 255};
    static constexpr RGB AMBER   = {255, 140,   0};
    static constexpr RGB LIME    = {150, 255,   0};
    static constexpr RGB PINK    = {255,  20, 147};
    static constexpr RGB TEAL    = {  0, 128, 128};
    static constexpr RGB BROWN   = {139,  69,  19};
    static constexpr RGB GOLD    = {255, 215,   0};
}

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

        enum class Indicator : uint16_t
        {
            NONE                 = 0,
            BLUETOOTH_CONNECTED  = 1 << 0,
            WIFI_CONNECTED       = 1 << 1,
            ALARM_TRIGGERED      = 1 << 2,
            ALARM_ARMED          = 1 << 3,
            BUTTON_HELD          = 1 << 4,
            TIME_SYNCED          = 1 << 5,
        };

        StatusLED(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
        void begin();
        void setState(State state);
        void setIndicator(Indicator indicator, bool active);
        void update();
        void setColor(uint8_t red, uint8_t green, uint8_t blue);

    private:
        uint8_t redPin, greenPin, bluePin;
        State   state;
        uint16_t indicatorFlags = 0;

        // Horloge du clignotement de l'état
        unsigned long lastToggleMs = 0;
        bool          ledOn        = false;

        // Séquenceur : -1 = on affiche l'état, >=0 = index dans la liste des indicateurs actifs
        static constexpr uint16_t STATE_PHASE_MS = 2000; // durée d'affichage de l'état avant de passer aux indicateurs
        static constexpr uint16_t PULSE_ON_MS    = 300;  // durée du flash d'un indicateur
        static constexpr uint16_t PULSE_GAP_MS   = 150;  // pause entre deux flashs

        unsigned long phaseStartMs           = 0;
        int8_t        currentIndicatorIndex  = -1;

        struct BlinkProfile {
            RGB color;
            uint16_t onMs, offMs;
            bool     blink;
        };

        static BlinkProfile profileFor(State s);
        static BlinkProfile colorFor(Indicator i);
        uint8_t collectActiveIndicators(Indicator* out, uint8_t maxCount) const;
    };

// ─── Implémentation ───────────────────────────────────────────────────────────

inline StatusLED::StatusLED(uint8_t r, uint8_t g, uint8_t b)
    : redPin(r), greenPin(g), bluePin(b), state(State::IDLE) {}

inline void StatusLED::begin()
{
    pinMode(redPin,   OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin,  OUTPUT);
    setState(State::IDLE);
}

inline void StatusLED::setState(State newState)
{
    if (newState == state) return;
    state                 = newState;
    lastToggleMs          = millis();
    ledOn                 = true;
    currentIndicatorIndex = -1;      // on repart sur la phase "état"
    phaseStartMs          = millis();
}

inline void StatusLED::setIndicator(Indicator indicator, bool active)
{
    uint16_t mask = static_cast<uint16_t>(indicator);
    if (active) indicatorFlags |= mask;
    else        indicatorFlags &= ~mask;
}

inline uint8_t StatusLED::collectActiveIndicators(Indicator* out, uint8_t maxCount) const
{
    uint8_t count = 0;
    for (uint8_t bit = 0; bit < 8 && count < maxCount; bit++)
    {
        uint8_t mask = 1 << bit;
        if (indicatorFlags & mask)
        {
            out[count++] = static_cast<Indicator>(mask);
        }
    }
    return count;
}

inline void StatusLED::update()
{
    unsigned long now = millis();

    Indicator active[8];
    uint8_t activeCount = collectActiveIndicators(active, 8);

    if (currentIndicatorIndex == -1)
    {
        // ── Phase "état" ──
        BlinkProfile p = profileFor(state);

        if (p.blink)
        {
            unsigned long elapsed = now - lastToggleMs;
            uint16_t      target  = ledOn ? p.onMs : p.offMs;
            if (elapsed >= target)
            {
                lastToggleMs = now;
                ledOn = !ledOn;
            }
            setColor(ledOn ? p.color.r : 0, ledOn ? p.color.g : 0, ledOn ? p.color.b : 0);
        }
        else
        {
            setColor(p.color.r, p.color.g, p.color.b);
        }

        if (activeCount > 0 && now - phaseStartMs >= STATE_PHASE_MS)
        {
            currentIndicatorIndex = -2;   // pause d'entrée avant le premier indicateur
            phaseStartMs = now;
        }
    }
    else if (currentIndicatorIndex == -2)
    {
        // ── Pause d'entrée avant le premier indicateur ──
        setColor(0, 0, 0);
        if (now - phaseStartMs >= PULSE_GAP_MS)
        {
            currentIndicatorIndex = 0;
            phaseStartMs = now;
        }
    }
    else
    {
        // ── Phase "indicateurs" : flash séquentiel de chaque flag actif ──
        if (currentIndicatorIndex >= activeCount)
        {
            currentIndicatorIndex = -1;
            phaseStartMs = now;
            lastToggleMs = now;
            ledOn        = true;
            return;
        }

        unsigned long elapsed = now - phaseStartMs;
        BlinkProfile  c       = colorFor(active[currentIndicatorIndex]);

        if (elapsed < PULSE_ON_MS)
        {
            setColor(c.color.r, c.color.g, c.color.b);
        }
        else if (elapsed < PULSE_ON_MS + PULSE_GAP_MS)
        {
            setColor(0, 0, 0);
        }
        else
        {
            currentIndicatorIndex++;
            phaseStartMs = now;
        }
    }
}

inline void StatusLED::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(redPin,   r);
    analogWrite(greenPin, g);
    analogWrite(bluePin,  b);
}

// ─── Profils par état ─────────────────────────────────────────────────────────
//                              R    G    B    onMs  offMs  blink
inline StatusLED::BlinkProfile StatusLED::profileFor(State s)
{
    switch (s)
    {
        case State::IDLE:                   return {LedColor::OFF,     Delay::MS_0,    Delay::MS_0,    false};
        case State::STARTING:               return {LedColor::WHITE,   Delay::MS_0,    Delay::MS_0,    false}; 
        case State::OK:                     return {LedColor::GREEN,   Delay::MS_100,  Delay::MS_5000, true};
        case State::WARNING_SENSOR:         return {LedColor::ORANGE,  Delay::MS_500,  Delay::MS_500,  true};
        case State::WARNING_COMMUNICATION:  return {LedColor::PURPLE,  Delay::MS_500,  Delay::MS_500,  true};
        case State::ERROR:                  return {LedColor::RED,     Delay::MS_500,  Delay::MS_500,  true};
        default:                            return {LedColor::OFF,     Delay::MS_0,    Delay::MS_0,    false};
    }
}

// ─── Couleurs des indicateurs (flash unique, pas de clignotement propre) ──────
//                                             R    G    B
inline StatusLED::BlinkProfile StatusLED::colorFor(Indicator i)
{
    switch (i)
    {
        case Indicator::BLUETOOTH_CONNECTED: return {LedColor::BLUE,   Delay::MS_0, Delay::MS_0, false};
        case Indicator::WIFI_CONNECTED:      return {LedColor::CYAN,   Delay::MS_0, Delay::MS_0, false};
        case Indicator::ALARM_ARMED:         return {LedColor::YELLOW, Delay::MS_0, Delay::MS_0, false};
        case Indicator::ALARM_TRIGGERED:     return {LedColor::RED,    Delay::MS_0, Delay::MS_0, false};
        case Indicator::BUTTON_HELD:         return {LedColor::TEAL,   Delay::MS_0, Delay::MS_0, false};
        case Indicator::TIME_SYNCED:         return {LedColor::BROWN,  Delay::MS_0, Delay::MS_0, false};
        default:                             return {LedColor::OFF,    Delay::MS_0, Delay::MS_0, false};
    }
}