/**
 * @file    status_led.hpp
 * @brief   RGB status LED interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-27
 */

#pragma once

#include <Arduino.h>
#include "rgb_led.hpp"

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
            TIME_SYNCED          = 1 << 5, //System time has been synchronized.
            LORAWAN_TX           = 1 << 6  //A LoRaWAN transmission has occurred recently.
        };

        /**
         * @brief Constructor for StatusLED.
         * @param redPin GPIO pin connected to the red LED channel.
         * @param greenPin GPIO pin connected to the green LED channel.
         * @param bluePin GPIO pin connected to the blue LED channel.
         */
        StatusLED(RgbLED& rgbLed);

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

    // ── Private members ───────────────────────────────────────────────────
    private:
        RgbLED& rgbLed;                       // RGB LED instance

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
        bool          indicatorsShown       = false; // Indicators already played in this off period

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

inline StatusLED::StatusLED(RgbLED& rgbLed)
    : rgbLed(rgbLed), state(State::IDLE) {}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void StatusLED::begin()
{
    rgbLed.begin();
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
    indicatorsShown       = false;
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

        // Long-off profiles (OK): indicators play inside the off period
        bool longOff = p.blink && p.offMs >= STATE_PHASE_MS;

        if (p.blink)
        {
            unsigned long elapsed = now - lastToggleMs;
            uint16_t target = ledOn ? p.onMs : p.offMs;

            if (elapsed >= target)
            {
                lastToggleMs = now;
                ledOn = !ledOn;
                if (ledOn) indicatorsShown = false;
            }

            if (ledOn) rgbLed.setColor(p.color);
            else       rgbLed.off();
        }
        else
        {
            rgbLed.setColor(p.color);
        }

        // Switch to the indicator sequence after the state phase
        bool startSequence = false;
        if (activeCount > 0)
        {
            if (longOff) startSequence = !ledOn && !indicatorsShown;
            else         startSequence = now - phaseStartMs >= STATE_PHASE_MS;
        }

        if (startSequence)
        {
            indicatorsShown       = true;
            currentIndicatorIndex = -2;
            phaseStartMs          = now;
        }
    }
    else if (currentIndicatorIndex == -2)
    {
        // ── Entry pause before the first indicator ──
        rgbLed.off();

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

            // Long-off profiles keep their own cycle timing
            BlinkProfile sp = profileFor(state);
            if (!(sp.blink && sp.offMs >= STATE_PHASE_MS))
            {
                lastToggleMs = now;
                ledOn        = true;
            }
            return;
        }

        unsigned long elapsed = now - phaseStartMs;
        BlinkProfile c = colorFor(active[currentIndicatorIndex]);

        if (elapsed < PULSE_ON_MS)
        {
            rgbLed.setColor(c.color);
        }
        else if (elapsed < PULSE_ON_MS + PULSE_GAP_MS)
        {
            rgbLed.off();
        }
        else
        {
            currentIndicatorIndex++;
            phaseStartMs = now;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// State Profiles
// ─────────────────────────────────────────────────────────────────────────────

inline StatusLED::BlinkProfile StatusLED::profileFor(State s)
{
    switch (s)
    {
        case State::IDLE:
            return {LEDColor::OFF, Delay::MS_0, Delay::MS_0, false};

        case State::STARTING:
            return {LEDColor::WHITE, Delay::MS_0, Delay::MS_0, false};

        case State::OK:
            return {LEDColor::GREEN, Delay::MS_100, Delay::MS_5000, true};

        case State::WARNING_SENSOR:
            return {LEDColor::ORANGE, Delay::MS_500, Delay::MS_500, true};

        case State::WARNING_COMMUNICATION:
            return {LEDColor::PURPLE, Delay::MS_500, Delay::MS_500, true};

        case State::ERROR:
            return {LEDColor::RED, Delay::MS_500, Delay::MS_500, true};

        default:
            return {LEDColor::OFF, Delay::MS_0, Delay::MS_0, false};
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
            return {LEDColor::BLUE, Delay::MS_0, Delay::MS_0, false};

        case Indicator::WIFI_CONNECTED:
            return {LEDColor::CYAN, Delay::MS_0, Delay::MS_0, false};

        case Indicator::ALARM_ARMED:
            return {LEDColor::YELLOW, Delay::MS_0, Delay::MS_0, false};

        case Indicator::ALARM_TRIGGERED:
            return {LEDColor::RED, Delay::MS_0, Delay::MS_0, false};

        case Indicator::BUTTON_HELD:
            return {LEDColor::TEAL, Delay::MS_0, Delay::MS_0, false};

        case Indicator::TIME_SYNCED:
            return {LEDColor::BROWN, Delay::MS_0, Delay::MS_0, false};
        
        case Indicator::LORAWAN_TX:
            return {LEDColor::MAGENTA, Delay::MS_0, Delay::MS_0, false};

        default:
            return {LEDColor::OFF, Delay::MS_0, Delay::MS_0, false};
    }
}