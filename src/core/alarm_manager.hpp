/**
 * @file    alarm_manager.hpp
 * @brief   Manages the device's alarm: arming, dismissing, and triggering the
 *          buzzer when the target epoch time is reached.
 * @author  Lucas Pichon
 * @date    2026-08-16
 */

#pragma once

#include <Arduino.h>
#include <functional>

#include "../core/clock.hpp"
#include "../interface/buzzer.hpp"

// ============================================================
// AlarmManager class definition
// ============================================================

class AlarmManager
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        using AlarmChangedCallback = std::function<void(bool armed, uint32_t targetEpoch)>;

        /**
         * @brief Constructor for AlarmManager.
         * @param buzzer Reference to the buzzer used to sound the alarm.
         * @param clock  Reference to the clock used to check the current time.
         */
        AlarmManager(Buzzer& buzzer, Clock& clock);

        /**
         * @brief Initialize the alarm manager with a starting state.
         * @param initialArmed      Whether the alarm should start armed.
         * @param initialTargetEpoch Initial target epoch time for the alarm.
         */
        void begin(bool initialArmed, uint32_t initialTargetEpoch);

        /**
         * @brief Update the alarm state. Must be called periodically (e.g. in the main loop).
         *        Handles triggering the buzzer when the target time is reached,
         *        stopping it after the maximum buzzing duration, and clearing
         *        the alarm once it has fired or become too late to trigger.
         */
        void update();

        /**
         * @brief Arm the alarm with a new target epoch time.
         * @param targetEpoch Target epoch time at which the alarm should ring.
         *                    Passing 0 stops the buzzer without arming a new alarm.
         */
        void setAlarm(uint32_t targetEpoch);

        /**
         * @brief Dismiss the current alarm, stopping the buzzer and clearing the alarm state.
         */
        void dismiss();

        /**
         * @brief Set the callback invoked whenever the alarm state changes (armed/target epoch).
         * @param callback Callback function to handle alarm state changes.
         */
        void onAlarmChanged(AlarmChangedCallback callback);

        /**
         * @brief Check whether the alarm is currently armed.
         * @return true if the alarm is armed, false otherwise.
         */
        bool isArmed() const;

        /**
         * @brief Check whether the alarm is currently ringing.
         * @return true if the buzzer is actively ringing for this alarm, false otherwise.
         */
        bool isRinging() const;

        /**
         * @brief Get the currently configured target epoch time.
         * @return The target epoch time, or 0 if no alarm is armed.
         */
        uint32_t getTargetEpoch() const;

    // ── Private members & helpers ───────────────────────────────────────────
    private:
        Buzzer& buzzer;   // Reference to the buzzer used to sound the alarm
        Clock& clock;     // Reference to the clock used to check the current time

        bool ringing = false;  // Whether the buzzer is currently ringing for this alarm

        AlarmChangedCallback alarmChangedCallback;  // Callback invoked when the alarm state changes

        bool armed = false;           // Whether the alarm is currently armed
        uint32_t targetEpoch = 0;     // Target epoch time at which the alarm should ring
        uint32_t buzzStartTime = 0;   // millis() timestamp at which the buzzer started ringing

        static constexpr uint32_t LATE_TOLERANCE_S = 600;         // Max delay (s) after target time during which the alarm can still ring
        static constexpr uint32_t MAX_BUZZER_DURATION_MS = 60000; // Max duration (ms) the buzzer is allowed to ring continuously

        /**
         * @brief Reset the alarm to its default (unarmed, no target) state and notify listeners.
         */
        void clearAlarm();

        /**
         * @brief Notify the registered callback (if any) of the current alarm state.
         */
        void notifyChanged();
};

// ============================================================
// implementations of AlarmManager methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline AlarmManager::AlarmManager(Buzzer& buzzer, Clock& clock)
    : buzzer(buzzer), clock(clock)
{

}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline void AlarmManager::begin(bool initialArmed, uint32_t initialTargetEpoch)
{
    // Store the provided initial state
    armed = initialArmed;
    targetEpoch = initialTargetEpoch;
}

// ─────────────────────────────────────────────────────────────────────────────
// Alarm Configuration
// ─────────────────────────────────────────────────────────────────────────────

inline void AlarmManager::setAlarm(uint32_t newTargetEpoch)
{
    if (newTargetEpoch == 0)
    {
        // A target of 0 simply stops any ongoing buzzing without arming a new alarm
        buzzer.stop();
        return;
    }

    // Store the new target time and arm the alarm
    targetEpoch = newTargetEpoch;
    armed = true;
    notifyChanged();
}

inline void AlarmManager::dismiss()
{
    // Stop the buzzer and reset the alarm state
    buzzer.stop();
    clearAlarm();
}

// ─────────────────────────────────────────────────────────────────────────────
// Update Loop
// ─────────────────────────────────────────────────────────────────────────────

inline void AlarmManager::update()
{
    if (!armed && buzzer.isActive())
    {
        // Safety cutoff: stop the buzzer if it has been ringing too long after being disarmed
        if (millis() - buzzStartTime >= MAX_BUZZER_DURATION_MS)
        {
            buzzer.stop();
            ringing = false;
        }
    }

    if (!armed || !clock.isSynchronized())
    {
        // Nothing to check if the alarm isn't armed or the clock isn't synchronized yet
        return;
    }

    uint32_t now = clock.now();

    if (now < targetEpoch)
    {
        // Target time not yet reached
        return;
    }

    uint32_t lateBy = now - targetEpoch;

    if (lateBy <= LATE_TOLERANCE_S)
    {
        // Still within tolerance: trigger the buzzer
        buzzer.start();
        buzzStartTime = millis();
        ringing = true;
    }

    // Whether triggered or too late, the alarm is consumed and cleared
    clearAlarm();
}

// ─────────────────────────────────────────────────────────────────────────────
// Internal Helpers
// ─────────────────────────────────────────────────────────────────────────────

inline void AlarmManager::clearAlarm()
{
    // Reset the alarm to its default state and notify listeners
    armed = false;
    ringing = false;
    targetEpoch = 0;
    notifyChanged();
}

inline void AlarmManager::notifyChanged()
{
    // Call the registered callback function, if any, with the current alarm state
    if (alarmChangedCallback)
    {
        alarmChangedCallback(armed, targetEpoch);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Callback Wiring
// ─────────────────────────────────────────────────────────────────────────────

inline void AlarmManager::onAlarmChanged(AlarmChangedCallback callback)
{
    // Store the provided callback function for alarm state changes
    alarmChangedCallback = callback;
}

// ─────────────────────────────────────────────────────────────────────────────
// State Accessors
// ─────────────────────────────────────────────────────────────────────────────

inline bool AlarmManager::isArmed() const
{
    return armed;
}

inline bool AlarmManager::isRinging() const
{
    return ringing;
}

inline uint32_t AlarmManager::getTargetEpoch() const
{
    return targetEpoch;
}