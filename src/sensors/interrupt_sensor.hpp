/**
 * @file    interrupt_sensor.hpp
 * @brief   Base class for digital sensors reporting events through a GPIO interrupt.
 * @author  Lucas Pichon
 * @date    2026-11-10
 */

#pragma once

#include "sensor.hpp"
#include "interrupt_input.hpp"


// ============================================================
// InterruptSensor class definition
// ============================================================

/**
 * @brief Base class for digital sensors whose output goes to an "active" level
 *        when something is detected (motion, vibration, obstacle...).
 *
 * A read reports the sensor as active if an event occurred since the last read,
 * or if the output is still at its active level (depending on useLevel).
 */
class InterruptSensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        InterruptInput input;  // Low-level access to the GPIO

    // ── Protected interface (for derived sensors) ───────────────────────────
    protected:

        /**
         * @param name        Sensor name.
         * @param bit         Bit of the sensor in the enabled-sensors mask.
         * @param pin         GPIO pin connected to the sensor digital output.
         * @param activeLevel Output level when something is detected (HIGH or LOW).
         * @param useLevel    Also report an event while the pin stays at its active level.
         */
        InterruptSensor(const char* name, SensorsBit bit, int pin, int activeLevel, bool useLevel);

        /**
         * @brief Report and consume the current detection state.
         * @return true if the sensor is considered active.
         */
        bool consumeEvent();

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Initialize the underlying GPIO interrupt.
         * @return true if the GPIO supports interrupts, false otherwise.
         */
        bool begin() override;
};


// ============================================================
// Implementations of InterruptSensor methods
// ============================================================

inline InterruptSensor::InterruptSensor(const char* name, SensorsBit bit, int pin, int activeLevel, bool useLevel)
    : Sensor(name, "", bit),
      input(pin, activeLevel, useLevel)
{
}

inline bool InterruptSensor::begin()
{
    initialized = input.begin();

    return initialized;
}

inline bool InterruptSensor::consumeEvent()
{
    return input.consumeEvent();
}
