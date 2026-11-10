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

        static constexpr unsigned long WINDOW_MS = 60; // Duration of the sound sampling window in milliseconds
        static constexpr uint8_t BUCKET_COUNT = 6;     // Number of buckets for sound sampling within the window
        static constexpr unsigned long BUCKET_MS = WINDOW_MS / BUCKET_COUNT; // Duration of each sound sampling bucket within the window
        static_assert(WINDOW_MS % BUCKET_COUNT == 0, "WINDOW_MS must be a multiple of BUCKET_COUNT");

        int bucketMax[BUCKET_COUNT];    // Maximum ADC value measured in each bucket
        int bucketMin[BUCKET_COUNT];    // Minimum ADC value measured in each bucket
        uint8_t currentBucket = 0;      // Index of the current sound sampling bucket
        unsigned long bucketStart = 0;  // Timestamp marking the beginning of the current sound sampling bucket

        // TODO: revoir le seuil avec la nouvelle fonction update + appel a handleSoundSensor à chaque boucle
        static constexpr int THRESHOLD = 200; // Minimum signal amplitude required to trigger sound detection

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

    for (int i = 0; i < BUCKET_COUNT; i++)
    {
        bucketMax[i] = 0;
        bucketMin[i] = 4095;
    }
    bucketStart = millis();

    initialized = true;

    return true;
}


// ─────────────────────────────────────────────────────────────────────────────
// Sound Detection Update
// ─────────────────────────────────────────────────────────────────────────────

inline void MAX9814Sensor::update()
{
    if (!initialized) return;

    // Read the current analog microphone signal
    int sample = analogRead(pin);

    // Update the current bucket with the new sample
    if (sample > bucketMax[currentBucket]) bucketMax[currentBucket] = sample;
    if (sample < bucketMin[currentBucket]) bucketMin[currentBucket] = sample;

    unsigned long now = millis();
    if (now - bucketStart >= BUCKET_MS)
    {
        // Evaluate the amplitude over the whole sliding window
        int maxValue = 0; 
        int minValue = 4095;

        for (uint8_t i = 0; i < BUCKET_COUNT; i++)
        {
            if (bucketMax[i] > maxValue) maxValue = bucketMax[i];
            if (bucketMin[i] < minValue) minValue = bucketMin[i];
        }

        if (maxValue - minValue > THRESHOLD)
        {
            soundDetected = true;
        }

        // Move to the next bucket, overwritting the oldest one
        currentBucket = (currentBucket + 1) % BUCKET_COUNT;
        bucketMax[currentBucket] = 0;
        bucketMin[currentBucket] = 4095;
        bucketStart = now;
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