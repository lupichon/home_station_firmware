/**
 * @file    sensor.hpp
 * @brief   MQ-2 analog gas sensor as seen by the application.
 * @author  Lucas Pichon
 * @date    2026-08-02
 */

#pragma once

#include <stdio.h>
#include "../sensor.hpp"
#include "../analog_input.hpp"


// ============================================================
// MQ2Sensor class definition
// ============================================================

class MQ2Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        AnalogInput input;  // Low-level access to the MQ-2 analog output

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @param pin Analog GPIO pin connected to the MQ-2 output.
         */
        MQ2Sensor(uint8_t pin);

        /**
         * @brief Configure the analog input.
         * @return true after the sensor has been initialized.
         */
        bool begin() override;

        /**
         * @brief Store the raw ADC value in the measurement.
         * @param m Measurement that receives the raw gas value.
         * @return true after the measurement has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the raw gas value for display.
         * @param m Measurement containing the raw gas value.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of MQ2Sensor methods
// ============================================================

inline MQ2Sensor::MQ2Sensor(uint8_t pin)
    : Sensor("MQ-2", "", MQ2_BIT),
      input(pin)
{
}

inline bool MQ2Sensor::begin()
{
    input.begin();

    initialized = true;

    return true;
}

inline bool MQ2Sensor::read(Measurement& m)
{
    m.gasRaw = input.read();

    return true;
}

inline const char* MQ2Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    snprintf(buffer, sizeof(buffer), "%s: %d", getName(), m.gasRaw);

    return buffer;
}
