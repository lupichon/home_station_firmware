/**
 * @file    buzzer.hpp
 * @brief   Escalating-pattern buzzer interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-08-16
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Type of buzzer wired to the device.
 */
enum class BuzzerType
{
    ACTIVE,   // Buzzer with its own internal oscillator (driven with digitalWrite)
    PASSIVE   // Buzzer requiring an external tone signal (driven with tone/noTone)
};

// ============================================================
// Buzzer class definition
// ============================================================

class Buzzer
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for Buzzer.
         * @param pin  GPIO pin connected to the buzzer.
         * @param type Type of buzzer (ACTIVE or PASSIVE).
         */
        Buzzer(uint8_t pin, BuzzerType type);

        /**
         * @brief Configure the GPIO pin and ensure the buzzer is silent.
         */
        void begin();

        /**
         * @brief Start the escalating beep pattern from the beginning.
         */
        void start();

        /**
         * @brief Stop the beep pattern immediately and silence the buzzer.
         */
        void stop();

        /**
         * @brief Advance the beep pattern state machine.
         *        Must be called regularly (e.g. in the main loop).
         */
        void update();

        /**
         * @brief Check whether the beep pattern is currently running.
         * @return true if the pattern is active, false otherwise.
         */
        bool isActive() const;

    // ── Private members ───────────────────────────────────────────────────
    private:
        uint8_t pin;              // GPIO pin connected to the buzzer
        BuzzerType type;          // Type of buzzer (ACTIVE or PASSIVE)

        bool running = false;         // Whether the beep pattern is currently running
        bool beepOn = false;          // Whether the buzzer is currently sounding within a cycle
        unsigned long lastToggle = 0; // Timestamp of the last on/off transition
        uint16_t cycleCount = 0;      // Number of completed beep cycles since start()

        // Constants defining the escalating beep pattern
        static constexpr uint16_t BEEP_ON_MS = 300;          // duration of each beep in milliseconds
        static constexpr uint16_t BEEP_OFF_MS_START = 700;   // initial pause duration between beeps in milliseconds
        static constexpr uint16_t BEEP_OFF_MS_MIN = 150;     // minimum pause duration between beeps in milliseconds
        static constexpr uint16_t ESCALATE_EVERY_CYCLES = 4; // accelerate the pattern every N completed beep cycles
        static constexpr uint16_t ESCALATE_STEP_MS = 100;    // amount by which the pause duration is reduced after each escalation
        static constexpr uint16_t TONE_FREQ_START = 1500;    // initial frequency for PASSIVE buzzer in Hz
        static constexpr uint16_t TONE_FREQ_MAX = 2500;      // maximum frequency for PASSIVE buzzer in Hz
        static constexpr uint16_t TONE_FREQ_STEP = 100;      // amount by which the frequency is increased after each escalation in Hz

        /**
         * @brief Drive the buzzer pin on or off, according to its type.
         * @param on true to sound the buzzer, false to silence it.
         */
        void setPin(bool on);

        /**
         * @brief Compute the current silence duration between beeps,
         *        which shrinks progressively as cycleCount increases.
         * @return The current off-duration in milliseconds.
         */
        uint16_t currentOffDuration() const;

        /**
         * @brief Compute the current tone frequency for a PASSIVE buzzer,
         *        which rises progressively as cycleCount increases.
         * @return The current tone frequency in Hz.
         */
        uint16_t currentToneFreq() const;
};

// ============================================================
// implementations of Buzzer methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline Buzzer::Buzzer(uint8_t pin, BuzzerType type) : pin(pin), type(type) {}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void Buzzer::begin()
{
    pinMode(pin, OUTPUT);
    setPin(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// Pattern Control
// ─────────────────────────────────────────────────────────────────────────────

inline void Buzzer::start()
{
    running = true;
    beepOn = false;
    cycleCount = 0;
    lastToggle = millis();
}

inline void Buzzer::stop()
{
    running = false;
    setPin(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// State Update
// ─────────────────────────────────────────────────────────────────────────────

inline void Buzzer::update()
{
    if (!running) return;

    unsigned long now = millis();
    unsigned long elapsed = now - lastToggle;

    if (beepOn)
    {
        // Buzzer is currently sounding: check if it's time to turn it off
        if (elapsed >= BEEP_ON_MS)
        {
            setPin(false);
            beepOn = false;
            lastToggle = now;
            cycleCount++;
        }
    }
    else
    {
        // Buzzer is currently silent: check if it's time to turn it on
        if (elapsed >= currentOffDuration())
        {
            setPin(true);
            beepOn = true;
            lastToggle = now;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Hardware Drive
// ─────────────────────────────────────────────────────────────────────────────

inline void Buzzer::setPin(bool on)
{
    if (type == BuzzerType::ACTIVE)
    {
        digitalWrite(pin, on ? HIGH : LOW);
    }
    else
    {
        if (on)
        {
            tone(pin, currentToneFreq());
        }
        else
        {
            noTone(pin);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Escalation Helpers
// ─────────────────────────────────────────────────────────────────────────────

inline uint16_t Buzzer::currentOffDuration() const
{
    uint16_t steps = cycleCount / ESCALATE_EVERY_CYCLES;
    int32_t off = (int32_t)BEEP_OFF_MS_START - steps * ESCALATE_STEP_MS;
    if (off < BEEP_OFF_MS_MIN) off = BEEP_OFF_MS_MIN;
    return (uint16_t)off;
}

inline uint16_t Buzzer::currentToneFreq() const
{
    uint16_t steps = cycleCount / ESCALATE_EVERY_CYCLES;
    uint32_t freq = TONE_FREQ_START + steps * TONE_FREQ_STEP;
    if (freq > TONE_FREQ_MAX) freq = TONE_FREQ_MAX;
    return (uint16_t)freq;
}

// ─────────────────────────────────────────────────────────────────────────────
// Status
// ─────────────────────────────────────────────────────────────────────────────

inline bool Buzzer::isActive() const
{
    return running;
}