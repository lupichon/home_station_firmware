/**
 * @file    sensor.hpp
 * @brief   Base interface for sensors used by the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-23-07
 */

#pragma once

#include <stdio.h>
#include "sensors_bit.hpp"
#include "../model/measurement.hpp"


// ============================================================
// Sensor class definition
// ============================================================

/**
 * @brief Application-level view of a sensor: identity, mask bit, and mapping
 *        of the hardware readings into a Measurement.
 */
class Sensor
{
    // ── Protected members ──────────────────────────────────────────────────
    protected:
        bool initialized = false;  // Indicates whether the sensor was initialized successfully

    // ── Private members ─────────────────────────────────────────────────────
    private:
        char sensorName[32];        // Name of the sensor
        char unit[16];              // Measurement unit of the sensor
        SensorsBit sensorBit;       // Bit of this sensor in the enabled-sensors mask

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for Sensor.
         * @param name Name assigned to the sensor.
         * @param unit Measurement unit associated with the sensor.
         * @param bit  Bit identifying the sensor in the enabled-sensors mask.
         */
        Sensor(const char* name, const char* unit, SensorsBit bit);

        virtual ~Sensor() = default;

        /**
         * @brief Get the name of the sensor.
         * @return Pointer to the null-terminated sensor name.
         */
        const char* getName() const;

        /**
         * @brief Get the measurement unit of the sensor.
         * @return Pointer to the null-terminated measurement unit.
         */
        const char* getUnit() const;

        /**
         * @brief Get the bit of this sensor in the enabled-sensors mask.
         * @return The bit corresponding to this sensor.
         */
        SensorsBit getBit() const;

        /**
         * @brief Check whether the sensor has been initialized.
         * @return true if the sensor is initialized, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Background sampling hook, called regularly while the sensor is enabled and initialized.
         *
         * Does nothing by default; override it for sensors that must be sampled
         * continuously, independently of read().
         */
        virtual void update() {}

        /**
         * @brief Initialize the sensor.
         * @return true if initialization was successful, false otherwise.
         */
        virtual bool begin() = 0;

        /**
         * @brief Read a measurement from the sensor.
         * @param m Measurement object that will receive the sensor data.
         * @return true if the measurement was successfully acquired,
         *         false otherwise.
         */
        virtual bool read(Measurement& m) = 0;

        /**
         * @brief Format a measurement value for display.
         * @param m Measurement to format.
         * @return Pointer to a null-terminated string containing the
         *         formatted value.
         */
        virtual const char* displayValue(const Measurement& m) const = 0;
};


// ============================================================
// Implementations of Sensor methods
// ============================================================

inline Sensor::Sensor(const char* name, const char* unit, SensorsBit bit)
    : sensorBit(bit)
{
    snprintf(sensorName, sizeof(sensorName), "%s", name);
    snprintf(this->unit, sizeof(this->unit), "%s", unit);
}

inline const char* Sensor::getName() const
{
    return sensorName;
}

inline const char* Sensor::getUnit() const
{
    return unit;
}

inline SensorsBit Sensor::getBit() const
{
    return sensorBit;
}

inline bool Sensor::isInitialized() const
{
    return initialized;
}
