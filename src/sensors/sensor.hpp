/**
 * @file    sensor.hpp
 * @brief   Base interface for sensors used by the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-23-07
 */

#pragma once

#include <stdio.h>
#include "../core/measurement.hpp"


// ============================================================
// Sensor class definition
// ============================================================

class Sensor
{
    // ── Protected members ──────────────────────────────────────────────────
    protected:
        bool initialized = false;  // Indicates whether the sensor was initialized successfully

    // ── Private members ─────────────────────────────────────────────────────
    private:
        char sensorName[32];        // Name of the sensor
        char unit[16];              // Measurement unit of the sensor

        static inline int sensorCount; // Number of currently existing sensors

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Default constructor for Sensor.
         *
         * Creates a sensor with an automatically generated name and no unit.
         */
        Sensor();

        /**
         * @brief Constructor for Sensor with a custom name.
         * @param name Name assigned to the sensor.
         */
        Sensor(const char* name);

        /**
         * @brief Constructor for Sensor with a custom name and unit.
         * @param name Name assigned to the sensor.
         * @param unit Measurement unit associated with the sensor.
         */
        Sensor(const char* name, const char* unit);

        /**
         * @brief Virtual destructor for the Sensor class.
         *
         * Decrements the global sensor instance counter when a sensor
         * object is destroyed.
         */
        virtual ~Sensor();

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
         * @brief Check whether the sensor has been initialized.
         * @return true if the sensor is initialized, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Get the number of currently existing sensor instances.
         * @return Number of active Sensor objects.
         */
        static int getSensorCount();

        /**
         * @brief Initialize the sensor.
         *
         * This method must be implemented by each derived sensor class.
         *
         * @return true if initialization was successful, false otherwise.
         */
        virtual bool begin() = 0;

        /**
         * @brief Read a measurement from the sensor.
         *
         * This method must be implemented by each derived sensor class.
         *
         * @param m Measurement object that will receive the sensor data.
         * @return true if the measurement was successfully acquired,
         *         false otherwise.
         */
        virtual bool read(Measurement& m) = 0;

        /**
         * @brief Format a measurement value for display.
         *
         * This method must be implemented by each derived sensor class.
         *
         * @param m Measurement to format.
         * @return Pointer to a null-terminated string containing the
         *         formatted value.
         */
        virtual const char* displayValue(const Measurement& m) const = 0;
};


// ============================================================
// Implementations of Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Default Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline Sensor::Sensor() : Sensor("", "")
{
    // Generate a default name using the current sensor count
    snprintf(sensorName, sizeof(sensorName), "Sensor_%d", sensorCount - 1);
}


// ─────────────────────────────────────────────────────────────────────────────
// Name Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline Sensor::Sensor(const char* name) : Sensor(name, "")
{

}


// ─────────────────────────────────────────────────────────────────────────────
// Name and Unit Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline Sensor::Sensor(const char* name, const char* unit)
{
    // Store the sensor name and measurement unit
    snprintf(sensorName, sizeof(sensorName), "%s", name);
    snprintf(this->unit, sizeof(this->unit), "%s", unit);

    // Register the new sensor instance
    Sensor::sensorCount++;
}


// ─────────────────────────────────────────────────────────────────────────────
// Sensor Information
// ─────────────────────────────────────────────────────────────────────────────

inline const char* Sensor::getName() const
{
    return sensorName;
}

inline const char* Sensor::getUnit() const
{
    return unit;
}

inline int Sensor::getSensorCount()
{
    return sensorCount;
}

inline bool Sensor::isInitialized() const
{
    return initialized;
}


// ─────────────────────────────────────────────────────────────────────────────
// Destructor
// ─────────────────────────────────────────────────────────────────────────────

inline Sensor::~Sensor()
{
    // Remove this instance from the global sensor count
    sensorCount--;
}