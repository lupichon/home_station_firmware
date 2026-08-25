/**
 * @file    status_led.hpp
 * @brief   RGB status LED interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-27
 */

#pragma once

#include <Arduino.h>


// ============================================================
// RGB color definition
// ============================================================

/**
 * @brief Represents an RGB color using 8-bit intensity values.
 */
struct RGB
{
    uint8_t r, g, b;
};


// ============================================================
// Timing constants
// ============================================================

/**
 * @brief Common delay values used by the status LED controller.
 */
namespace Delay
{
    static constexpr unsigned long MS_0    = 0;
    static constexpr unsigned long MS_10   = 10;
    static constexpr unsigned long MS_100  = 100;
    static constexpr unsigned long MS_500  = 500;
    static constexpr unsigned long MS_1000 = 1000;
    static constexpr unsigned long MS_5000 = 5000;
}


// ============================================================
// LED color definitions
// ============================================================

/**
 * @brief Predefined RGB colors used by the status LED.
 */
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


// ============================================================
// StatusLED class definition
// ============================================================

class StatusLED
{
    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Represents the global operating state displayed by the LED.
         */
        enum class State
        {
            IDLE,                   //Device is idle.
            STARTING,               //Device is starting.
            OK,                     //Device is operating normally.
            WARNING_SENSOR,         //A sensor-related warning is active.
            WARNING_COMMUNICATION,  //A communication-related warning is active.
            ERROR                   //A critical error is active.
        };

        /**
         * @brief Represents additional status indicators displayed sequentially.
         *
         * Each indicator is represented by a bit flag, allowing several
         * indicators to be active simultaneously.
         */
        enum class Indicator : uint16_t
        {
            NONE                 = 0,      //No indicator is active.
            BLUETOOTH_CONNECTED  = 1 << 0, //Bluetooth connection is active.
            WIFI_CONNECTED       = 1 << 1, //Wi-Fi connection is active.
            ALARM_TRIGGERED      = 1 << 2, //An alarm has been triggered.
            ALARM_ARMED          = 1 << 3, //The alarm system is armed.
            BUTTON_HELD          = 1 << 4, //A button is currently held.
            TIME_SYNCED          = 1 << 5  //System time has been synchronized.
        };

        /**
         * @brief Constructor for StatusLED.
         * @param redPin GPIO pin connected to the red LED channel.
         * @param greenPin GPIO pin connected to the green LED channel.
         * @param bluePin GPIO pin connected to the blue LED channel.
         */
        StatusLED(uint8_t redPin, uint8_t greenPin, uint8_t bluePin);

        /**
         * @brief Configure the RGB LED pins and initialize the LED state.
         */
        void begin();

        /**
         * @brief Set the main operating state displayed by the LED.
         * @param state New status state to display.
         */
        void setState(State state);

        /**
         * @brief Enable or disable an additional status indicator.
         * @param indicator Indicator to enable or disable.
         * @param active true to enable the indicator, false to disable it.
         */
        void setIndicator(Indicator indicator, bool active);

        /**
         * @brief Update the LED display and handle timing of states and indicators.
         *
         * Must be called regularly, for example from the main loop.
         */
        void update();

        /**
         * @brief Set the RGB LED output color directly.
         * @param red Red channel intensity from 0 to 255.
         * @param green Green channel intensity from 0 to 255.
         * @param blue Blue channel intensity from 0 to 255.
         */
        void setColor(uint8_t red, uint8_t green, uint8_t blue);

    // ── Private members ───────────────────────────────────────────────────
    private:
        uint8_t redPin, greenPin, bluePin;  // GPIO pins for the RGB channels

        State state;                        // Current main operating state
        uint16_t indicatorFlags = 0;        // Bitmask of active indicators

        // Blink timing
        unsigned long lastToggleMs = 0;     // Timestamp of the last LED toggle
        bool          ledOn        = false; // Current blink output state

        static constexpr uint16_t STATE_PHASE_MS = 5000; // State display duration
        static constexpr uint16_t PULSE_ON_MS    = 500;  // Indicator flash duration
        static constexpr uint16_t PULSE_GAP_MS   = 200;  // Pause between flashes

        // Indicator sequence state
        unsigned long phaseStartMs          = 0;
        int8_t        currentIndicatorIndex = -1;

        /**
         * @brief Defines the display behavior of an LED state or indicator.
         */
        struct BlinkProfile
        {
            RGB color;          // Color displayed by the LED
            uint16_t onMs;      // ON duration for blinking
            uint16_t offMs;     // OFF duration for blinking
            bool blink;         // true if the color should blink
        };

        /**
         * @brief Get the LED display profile associated with a main state.
         * @param s State for which the profile is requested.
         * @return Blink profile containing the color and timing information.
         */
        static BlinkProfile profileFor(State s);

        /**
         * @brief Get the display profile associated with an indicator.
         * @param i Indicator for which the profile is requested.
         * @return Blink profile containing the indicator color.
         */
        static BlinkProfile colorFor(Indicator i);

        /**
         * @brief Collect all currently active indicators.
         * @param out Output array receiving the active indicators.
         * @param maxCount Maximum number of indicators that can be stored.
         * @return Number of active indicators collected.
         */
        uint8_t collectActiveIndicators(Indicator* out, uint8_t maxCount) const;
    };


// ============================================================
// Implementations of StatusLED methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline StatusLED::StatusLED(uint8_t r, uint8_t g, uint8_t b)
    : redPin(r), greenPin(g), bluePin(b), state(State::IDLE) {}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::begin()
{
    // Configure the RGB channels as outputs
    pinMode(redPin,   OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin,  OUTPUT);

    // Start in the idle state
    setState(State::IDLE);
}


// ─────────────────────────────────────────────────────────────────────────────
// State Management
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::setState(State newState)
{
    if (newState == state) return;

    state                 = newState;
    lastToggleMs          = millis();
    ledOn                 = true;
    currentIndicatorIndex = -1;
    phaseStartMs          = millis();
}


// ─────────────────────────────────────────────────────────────────────────────
// Indicator Management
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::setIndicator(Indicator indicator, bool active)
{
    uint16_t mask = static_cast<uint16_t>(indicator);

    if (active)
    {
        indicatorFlags |= mask;
    }
    else
    {
        indicatorFlags &= ~mask;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Active Indicator Collection
// ─────────────────────────────────────────────────────────────────────────────

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


// ─────────────────────────────────────────────────────────────────────────────
// LED State Update
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::update()
{
    unsigned long now = millis();

    Indicator active[8];
    uint8_t activeCount = collectActiveIndicators(active, 8);

    if (currentIndicatorIndex == -1)
    {
        // ── State display phase ──
        BlinkProfile p = profileFor(state);

        if (p.blink)
        {
            unsigned long elapsed = now - lastToggleMs;
            uint16_t target = ledOn ? p.onMs : p.offMs;

            if (elapsed >= target)
            {
                lastToggleMs = now;
                ledOn = !ledOn;
            }

            setColor(
                ledOn ? p.color.r : 0,
                ledOn ? p.color.g : 0,
                ledOn ? p.color.b : 0
            );
        }
        else
        {
            setColor(p.color.r, p.color.g, p.color.b);
        }

        // Switch to the indicator sequence after the state phase
        if (activeCount > 0 && now - phaseStartMs >= STATE_PHASE_MS)
        {
            currentIndicatorIndex = -2;
            phaseStartMs = now;
        }
    }
    else if (currentIndicatorIndex == -2)
    {
        // ── Entry pause before the first indicator ──
        setColor(0, 0, 0);

        if (now - phaseStartMs >= PULSE_GAP_MS)
        {
            currentIndicatorIndex = 0;
            phaseStartMs = now;
        }
    }
    else
    {
        // ── Indicator phase: sequential flash of each active indicator ──
        if (currentIndicatorIndex >= activeCount)
        {
            currentIndicatorIndex = -1;
            phaseStartMs = now;
            lastToggleMs = now;
            ledOn        = true;
            return;
        }

        unsigned long elapsed = now - phaseStartMs;
        BlinkProfile c = colorFor(active[currentIndicatorIndex]);

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


// ─────────────────────────────────────────────────────────────────────────────
// Color Output
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    // Set the PWM intensity of each RGB channel
    analogWrite(redPin,   r);
    analogWrite(greenPin, g);
    analogWrite(bluePin,  b);
}


// ─────────────────────────────────────────────────────────────────────────────
// State Profiles
// ─────────────────────────────────────────────────────────────────────────────

inline StatusLED::BlinkProfile StatusLED::profileFor(State s)
{
    switch (s)
    {
        case State::IDLE:
            return {LedColor::OFF, Delay::MS_0, Delay::MS_0, false};

        case State::STARTING:
            return {LedColor::WHITE, Delay::MS_0, Delay::MS_0, false};

        case State::OK:
            return {LedColor::GREEN, Delay::MS_100, Delay::MS_5000, true};

        case State::WARNING_SENSOR:
            return {LedColor::ORANGE, Delay::MS_500, Delay::MS_500, true};

        case State::WARNING_COMMUNICATION:
            return {LedColor::PURPLE, Delay::MS_500, Delay::MS_500, true};

        case State::ERROR:
            return {LedColor::RED, Delay::MS_500, Delay::MS_500, true};

        default:
            return {LedColor::OFF, Delay::MS_0, Delay::MS_0, false};
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Indicator Profiles
// ─────────────────────────────────────────────────────────────────────────────

inline StatusLED::BlinkProfile StatusLED::colorFor(Indicator i)
{
    switch (i)
    {
        case Indicator::BLUETOOTH_CONNECTED:
            return {LedColor::BLUE, Delay::MS_0, Delay::MS_0, false};

        case Indicator::WIFI_CONNECTED:
            return {LedColor::CYAN, Delay::MS_0, Delay::MS_0, false};

        case Indicator::ALARM_ARMED:
            return {LedColor::YELLOW, Delay::MS_0, Delay::MS_0, false};

        case Indicator::ALARM_TRIGGERED:
            return {LedColor::RED, Delay::MS_0, Delay::MS_0, false};

        case Indicator::BUTTON_HELD:
            return {LedColor::TEAL, Delay::MS_0, Delay::MS_0, false};

        case Indicator::TIME_SYNCED:
            return {LedColor::BROWN, Delay::MS_0, Delay::MS_0, false};

        default:
            return {LedColor::OFF, Delay::MS_0, Delay::MS_0, false};
    }
}