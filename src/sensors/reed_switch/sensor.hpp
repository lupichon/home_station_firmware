/**
 * @file    sensor.hpp
 * @brief   Door state received from the remote reed switch node (over Bluetooth).
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stdio.h>
#include "../sensor.hpp"


// ============================================================
// ReedSwitchSensor class definition
// ============================================================

/**
 * @brief Latches "door opened" events pushed by the remote node, so that an opening
 *        shorter than the read interval is not lost.
 */
class ReedSwitchSensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        volatile bool doorOpened = false;  // Set from the Bluetooth callback, cleared by read()

    // ── Public interface ────────────────────────────────────────────────────
    public:

        ReedSwitchSensor();

        /**
         * @brief Clear any pending event.
         * @return true always: the sensor has no local hardware.
         */
        bool begin() override;

        /**
         * @brief Record the door state received from the remote node.
         * @param open true if the door is open.
         */
        void onStateReceived(bool open);

        /**
         * @brief Read and consume the "door opened" event.
         * @param m Measurement that receives the door state.
         * @return true always.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the door state of a measurement for display.
         * @param m Measurement containing the door state.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of ReedSwitchSensor methods
// ============================================================

inline ReedSwitchSensor::ReedSwitchSensor()
    : Sensor("Reed switch", "", REED_SWITCH_BIT)
{
}

inline bool ReedSwitchSensor::begin()
{
    doorOpened = false;
    initialized = true;

    return true;
}

inline void ReedSwitchSensor::onStateReceived(bool open)
{
    if (open)
    {
        doorOpened = true;
    }
}

inline bool ReedSwitchSensor::read(Measurement& m)
{
    m.doorOpen = doorOpened;

    // Reset the event flag after it has been consumed
    doorOpened = false;

    return true;
}

inline const char* ReedSwitchSensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s: %s", getName(), m.doorOpen ? "Door opened" : "Door closed");

    return buffer;
}
