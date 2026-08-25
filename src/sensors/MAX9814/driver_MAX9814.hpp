/**
 * @file    driver_MAX9814.hpp
 * @brief   MAX9814 analog microphone sensor interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-27
 */

#pragma once

#include <Arduino.h>
#include "../sensor.hpp"


// ============================================================
// MAX9814Sensor class definition
// ============================================================

class MAX9814Sensor : public Sensor
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        int pin;  // Analog GPIO pin connected to the MAX9814 output

        bool soundDetected = false; // Indicates whether sound has been detected since the last read

        int signalMax = 0;     // Maximum ADC value measured during the current window
        int signalMin = 4095;  // Minimum ADC value measured during the current window

        unsigned long windowStart = 0; // Timestamp marking the beginning of the current sampling window

        static constexpr unsigned long WINDOW_MS = 60; // Duration of the sound sampling window in milliseconds

        static constexpr int THRESHOLD = 500; // Minimum signal amplitude required to trigger sound detection

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Constructor for MAX9814Sensor.
         * @param pin Analog GPIO pin connected to the MAX9814 output.
         */
        MAX9814Sensor(int pin);

        /**
         * @brief Update the sound detection state.
         */
        void update();

        /**
         * @brief Initialize the MAX9814 sensor.
         * @return true after the sensor has been initialized.
         */
        bool begin() override;

        /**
         * @brief Read and consume the current sound detection event.
         * @param m Measurement object that will receive the sound state.
         * @return true after the sound state has been read.
         */
        bool read(Measurement& m) override;

        /**
         * @brief Format the current sound detection state for display.
         * @param m Measurement containing the sound detection state.
         * @return Pointer to a formatted null-terminated string.
         */
        const char* displayValue(const Measurement& m) const override;
};


// ============================================================
// Implementations of MAX9814Sensor methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline MAX9814Sensor::MAX9814Sensor(int pin)
    : Sensor("MAX9814"),
      pin(pin)
{
}


// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool MAX9814Sensor::begin()
{
    // Configure the MAX9814 output as an analog input
    pinMode(pin, INPUT);

    // Configure the ADC for 12-bit resolution (0-4095)
    analogReadResolution(12);

    initialized = true;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Sound Detection Update
// ─────────────────────────────────────────────────────────────────────────────

inline void MAX9814Sensor::update()
{
    // Read the current analog microphone signal
    int sample = analogRead(pin);

    // Track the signal range during the current sampling window
    if (sample > signalMax)
    {
        signalMax = sample;
    }

    if (sample < signalMin)
    {
        signalMin = sample;
    }

    // Evaluate the signal once the sampling window has elapsed
    if (millis() - windowStart >= WINDOW_MS)
    {
        int amplitude = signalMax - signalMin;

        // Detect sound if the signal amplitude exceeds the threshold
        soundDetected = (amplitude > THRESHOLD);

        // Reset the sampling window
        signalMax = 0;
        signalMin = 4095;
        windowStart = millis();
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// Measurement Reading
// ─────────────────────────────────────────────────────────────────────────────

inline bool MAX9814Sensor::read(Measurement& m)
{
    // Copy the sound detection event into the Measurement structure
    m.sound = soundDetected;

    // Reset the event flag after it has been consumed
    soundDetected = false;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Display Formatting
// ─────────────────────────────────────────────────────────────────────────────

inline const char* MAX9814Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];

    // Format the sensor name and sound detection status
    snprintf(
        buffer,
        sizeof(buffer),
        "%s: %s",
        getName(),
        m.sound ? "Sound detected" : "No sound"
    );

    return buffer;
}