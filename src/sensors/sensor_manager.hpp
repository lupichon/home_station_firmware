/**
 * @file    sensor_manager.hpp
 * @brief   Decides which sensors run and when, and tracks their health.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include "sensor.hpp"


// ============================================================
// SensorManager class definition
// ============================================================

class SensorManager
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        static constexpr size_t MAX_SENSORS = 16;       // Size of the enabled-sensors mask
        static constexpr int MAX_CONSECUTIVE_FAILED_READS = 3;  // Failed reads before a re-initialization

        Sensor* const* sensors;  // Registered sensors (not owned)
        size_t sensorCount;

        bool lastReadOk[MAX_SENSORS]  = {false};
        int  failureCount[MAX_SENSORS] = {0};

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @param sensors Array of sensors, which must outlive the manager. Order does not matter.
         * @param count   Number of sensors (at most 16).
         */
        SensorManager(Sensor* const* sensors, size_t count);

        /**
         * @brief Initialize every enabled sensor.
         * @param enabledMask Bitmask of enabled sensors.
         */
        void begin(uint16_t enabledMask);

        /**
         * @brief Run the background sampling of every enabled and initialized sensor.
         * @param enabledMask Bitmask of enabled sensors.
         */
        void update(uint16_t enabledMask);

        /**
         * @brief Read every enabled sensor into the measurement.
         *
         * A sensor that is not initialized is initialized again, and one that fails
         * several reads in a row is re-initialized.
         *
         * @param m           Measurement that receives the values.
         * @param enabledMask Bitmask of enabled sensors.
         * @return true if every enabled sensor was read successfully.
         */
        bool readAll(Measurement& m, uint16_t enabledMask);

        size_t count() const;
        const Sensor& at(size_t index) const;
        bool isEnabled(size_t index, uint16_t enabledMask) const;
        bool lastReadSucceeded(size_t index) const;
};


// ============================================================
// Implementations of SensorManager methods
// ============================================================

inline SensorManager::SensorManager(Sensor* const* sensors, size_t count)
    : sensors(sensors),
      sensorCount(count < MAX_SENSORS ? count : MAX_SENSORS)
{
}

inline void SensorManager::begin(uint16_t enabledMask)
{
    for (size_t i = 0; i < sensorCount; i++)
    {
        if (isEnabled(i, enabledMask))
        {
            sensors[i]->begin();
        }
    }
}

inline void SensorManager::update(uint16_t enabledMask)
{
    for (size_t i = 0; i < sensorCount; i++)
    {
        if (isEnabled(i, enabledMask) && sensors[i]->isInitialized())
        {
            sensors[i]->update();
        }
    }
}

inline bool SensorManager::readAll(Measurement& m, uint16_t enabledMask)
{
    bool success = true;

    for (size_t i = 0; i < sensorCount; i++)
    {
        if (!isEnabled(i, enabledMask))
        {
            lastReadOk[i] = false;
            continue;
        }

        Sensor& sensor = *sensors[i];

        if (!sensor.isInitialized())
        {
            // Not initialized: retry begin() on every cycle
            sensor.begin();
            lastReadOk[i] = false;
            failureCount[i] = 0;
            success = false;
            continue;
        }

        bool ok = sensor.read(m);
        lastReadOk[i] = ok;
        success &= ok;

        if (ok)
        {
            failureCount[i] = 0;
        }
        else if (++failureCount[i] >= MAX_CONSECUTIVE_FAILED_READS)
        {
            sensor.begin();
            failureCount[i] = 0;
        }
    }

    return success;
}

inline size_t SensorManager::count() const
{
    return sensorCount;
}

inline const Sensor& SensorManager::at(size_t index) const
{
    return *sensors[index];
}

inline bool SensorManager::isEnabled(size_t index, uint16_t enabledMask) const
{
    return isSensorEnabled(enabledMask, sensors[index]->getBit());
}

inline bool SensorManager::lastReadSucceeded(size_t index) const
{
    return lastReadOk[index];
}
