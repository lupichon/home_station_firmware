/**
 * @file    measurement.hpp
 * @brief   Defines the sensor measurement data structure and its associated
 *          transmission status, along with a helper to reset a measurement.
 * @author  Lucas Pichon
 * @date    2026-07-23
 */

#pragma once

#include <math.h>
#include <stdint.h>

// ============================================================
// Measurement struct definition
// ============================================================

/**
 * @brief Holds a single set of sensor readings taken at a given timestamp.
 *        Fields default to NAN (float fields) or false (boolean fields) to
 *        represent "no reading yet" / "not applicable".
 */
struct Measurement
{
    uint32_t timestamp = 0;         // Epoch time at which the measurement was taken
    uint32_t acquiredMs = 0;        // millis() at acquisition, used to measure the age of the reading

    float temperature  = NAN;       // Temperature reading (°C)
    float humidity     = NAN;       // Relative humidity reading (%)
    float luminosity   = NAN;       // Luminosity reading (lux)
    float pressure     = NAN;       // Atmospheric pressure reading (hPa)
    uint16_t co2       = 0;       // CO2 concentration reading (ppm)
    uint16_t gasRaw    = 0;       // Raw gas sensor reading
    uint16_t vocIndex  = 0;       // VOC (volatile organic compounds) index
    uint16_t noxIndex  = 0;       // NOx (nitrogen oxides) index

    bool motion        = false;     // Whether motion was detected
    bool sound         = false;     // Whether sound was detected
    bool obstacle      = false;     // Whether an obstacle was detected
    bool vibration     = false;     // Whether vibration was detected
};

// ============================================================
// Measurement accumulator
// ============================================================

/**
 * @brief Running sum and sample count for one numeric field.
 */
struct RunningAverage
{
    float    sum   = 0.0f;
    uint32_t count = 0;

    void add(float value)
    {
        if (isnan(value)) return;
        sum += value;
        count++;
    }

    float get() const { return count ? sum / count : NAN; }
    uint32_t getCount() const { return count; }
    float getSum() const { return sum; }
    void  reset()     { sum = 0.0f; count = 0; }
};

/**
 * @brief Accumulates Measurements between two transmissions and produces
 *        their average. Booleans are OR-ed (event seen at least once).
 *        Integer fields equal to 0 are treated as "no reading" and skipped.
 */
struct MeasurementAccumulator
{
    uint32_t lastTimestamp = 0;
    uint32_t lastAcquiredMs = 0;  

    RunningAverage temperature, humidity, luminosity, pressure;
    RunningAverage co2, gasRaw, vocIndex, noxIndex;

    bool motion = false, sound = false, obstacle = false, vibration = false;

    void add(const Measurement& m)
    {
        lastTimestamp = m.timestamp;
        lastAcquiredMs = m.acquiredMs;

        temperature.add(m.temperature);
        humidity.add(m.humidity);
        luminosity.add(m.luminosity);
        pressure.add(m.pressure);

        co2.add(m.co2);
        gasRaw.add(m.gasRaw);
        vocIndex.add(m.vocIndex);
        noxIndex.add(m.noxIndex);

        motion    |= m.motion;
        sound     |= m.sound;
        obstacle  |= m.obstacle;
        vibration |= m.vibration;
    }

    bool empty() const
    {
        return temperature.count == 0 && humidity.count == 0 &&
               luminosity.count  == 0 && pressure.count == 0 &&
               co2.count == 0 && gasRaw.count == 0 &&
               vocIndex.count == 0 && noxIndex.count == 0;
    }

    Measurement average() const
    {
        Measurement m;
        m.timestamp   = lastTimestamp;  
        m.acquiredMs  = lastAcquiredMs;
        m.temperature = temperature.get();
        m.humidity    = humidity.get();
        m.luminosity  = luminosity.get();
        m.pressure    = pressure.get();
        m.co2         = co2.count      ? (uint16_t)lroundf(co2.get())      : 0;
        m.gasRaw      = gasRaw.count   ? (uint16_t)lroundf(gasRaw.get())   : 0;
        m.vocIndex    = vocIndex.count ? (uint16_t)lroundf(vocIndex.get()) : 0;
        m.noxIndex    = noxIndex.count ? (uint16_t)lroundf(noxIndex.get()) : 0;
        m.motion      = motion;
        m.sound       = sound;
        m.obstacle    = obstacle;
        m.vibration   = vibration;
        return m;
    }

    void reset() { *this = MeasurementAccumulator(); }

    void printDebugInfo() const
    {   
        Serial.print("Temperature: ");  Serial.print("Count: "); Serial.print(temperature.getCount());  Serial.print(" Sum: "); Serial.print(temperature.getSum()); Serial.print(" Mean: "); Serial.println(temperature.get());
        Serial.print("Humidity: ");     Serial.print("Count: "); Serial.print(humidity.getCount());     Serial.print(" Sum: "); Serial.print(humidity.getSum());    Serial.print(" Mean: "); Serial.println(humidity.get());
        Serial.print("Luminosity: ");   Serial.print("Count: "); Serial.print(luminosity.getCount());   Serial.print(" Sum: "); Serial.print(luminosity.getSum());  Serial.print(" Mean: "); Serial.println(luminosity.get());
        Serial.print("Pressure: ");     Serial.print("Count: "); Serial.print(pressure.getCount());     Serial.print(" Sum: "); Serial.print(pressure.getSum());    Serial.print(" Mean: "); Serial.println(pressure.get());
        Serial.print("CO2: ");          Serial.print("Count: "); Serial.print(co2.getCount());          Serial.print(" Sum: "); Serial.print(co2.getSum());         Serial.print(" Mean: "); Serial.println(co2.get());
        Serial.print("Gas Raw: ");      Serial.print("Count: "); Serial.print(gasRaw.getCount());       Serial.print(" Sum: "); Serial.print(gasRaw.getSum());      Serial.print(" Mean: "); Serial.println(gasRaw.get());
        Serial.print("VOC Index: ");    Serial.print("Count: "); Serial.print(vocIndex.getCount());     Serial.print(" Sum: "); Serial.print(vocIndex.getSum());    Serial.print(" Mean: "); Serial.println(vocIndex.get());
        Serial.print("NOx Index: ");    Serial.print("Count: "); Serial.print(noxIndex.getCount());     Serial.print(" Sum: "); Serial.print(noxIndex.getSum());    Serial.print(" Mean: "); Serial.println(noxIndex.get());
        Serial.print("Motion: ");       Serial.println(motion);
        Serial.print("Sound: ");        Serial.println(sound);
        Serial.print("Obstacle: ");     Serial.println(obstacle);
        Serial.print("Vibration: ");    Serial.println(vibration);
    }
};

// ============================================================
// MeasurementStatus struct definition
// ============================================================

/**
 * @brief Reports whether a measurement was successfully read and/or
 *        successfully transmitted over each communication channel.
 */
struct MeasurementStatus
{
    bool sensorOK;      // Whether the sensor reading itself succeeded
    bool bluetoothOK;   // Whether the measurement was sent over Bluetooth successfully
    bool lorawanOK;     // Whether the measurement was sent over LoRaWAN successfully
};

// ============================================================
// Helpers
// ============================================================

/**
 * @brief Reset a Measurement to its default "empty" state (no timestamp,
 *        NAN readings, all flags false).
 * @param measurement Measurement instance to clear, in place.
 */
inline void clearMeasurement(Measurement& measurement)
{
    measurement.timestamp   = 0;
    measurement.acquiredMs  = 0;
    measurement.temperature = NAN;
    measurement.humidity    = NAN;
    measurement.co2         = 0;
    measurement.luminosity  = NAN;
    measurement.pressure    = NAN;
    measurement.gasRaw      = 0;
    measurement.vocIndex    = 0;
    measurement.noxIndex    = 0;
    measurement.motion      = false;
    measurement.sound       = false;
    measurement.obstacle    = false;
    measurement.vibration   = false;
}