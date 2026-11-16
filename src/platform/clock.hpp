/**
 * @file    clock.hpp
 * @brief   Software clock synchronized from an external epoch time (e.g. via
 *          LoRaWAN or Bluetooth), tracking elapsed time with millis() and
 *          providing local time component extraction.
 * @author  Lucas Pichon
 * @date    2026-08-16
 */

#pragma once

#include <Arduino.h>
#include <time.h>

// ============================================================
// Clock class definition
// ============================================================

class Clock
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Synchronize the clock with a reference epoch time.
         *        The internal reference is used together with millis() to keep
         *        track of elapsed time between synchronizations.
         * @param epoch Current epoch time (seconds since 1970-01-01T00:00:00Z) to synchronize to.
         */
        void sync(uint32_t epoch);

        /**
         * @brief Check whether the clock has been synchronized at least once.
         * @return true if the clock is synchronized, false otherwise.
         */
        bool isSynchronized() const;

        /**
         * @brief Get the number of seconds elapsed since the clock was last synchronized.
         * @return Elapsed time in seconds since synchronization, or 0 if never synchronized.
         */
        uint32_t isSynchronizedSince() const;

        /**
         * @brief Configure the UTC offset used for local time calculations.
         * @param offset UTC offset in hours (can be negative).
         */
        void configure(int8_t offset);

        /**
         * @brief Get the current epoch time, computed from the last synchronization and millis().
         * @return Current epoch time, or 0 if the clock has not been synchronized.
         */
        uint32_t now() const;

        /**
         * @brief Extract the hour (0-23) from an epoch time, adjusted for the configured UTC offset.
         * @param epoch Epoch time to convert.
         * @return Hour of the day (0-23).
         */
        uint8_t hour(uint32_t epoch) const;

        /**
         * @brief Extract the minute (0-59) from an epoch time.
         * @param epoch Epoch time to convert.
         * @return Minute of the hour (0-59).
         */
        uint8_t minute(uint32_t epoch) const;

        /**
         * @brief Extract the second (0-59) from an epoch time.
         * @param epoch Epoch time to convert.
         * @return Second of the minute (0-59).
         */
        uint8_t second(uint32_t epoch) const;

        /**
         * @brief Extract the day of the month (1-31) from an epoch time (UTC).
         * @param epoch Epoch time to convert.
         * @return Day of the month (1-31).
         */
        uint8_t day(uint32_t epoch) const;

        /**
         * @brief Extract the month (1-12) from an epoch time (UTC).
         * @param epoch Epoch time to convert.
         * @return Month of the year (1-12).
         */
        uint8_t month(uint32_t epoch) const;

        /**
         * @brief Extract the year from an epoch time (UTC).
         * @param epoch Epoch time to convert.
         * @return Year (e.g. 2026).
         */
        uint16_t year(uint32_t epoch) const;

    // ── Private members ─────────────────────────────────────────────────────
    private:
        bool synchronized = false;   // Whether the clock has been synchronized at least once

        uint32_t referenceEpoch = 0;   // Epoch time captured at the last synchronization
        uint32_t referenceMillis = 0;  // millis() value captured at the last synchronization

        int8_t utcOffset = 0;   // UTC offset (in hours) used for local time calculations
};

// ============================================================
// implementations of Clock methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Synchronization
// ─────────────────────────────────────────────────────────────────────────────

inline void Clock::sync(uint32_t epoch)
{
    // Store the reference epoch and the corresponding millis() timestamp
    referenceEpoch = epoch;
    referenceMillis = millis();
    synchronized = true;
}

inline bool Clock::isSynchronized() const
{
    return synchronized;
}

inline uint32_t Clock::isSynchronizedSince() const
{
    // Return the elapsed time since the last synchronization, or 0 if never synchronized
    return isSynchronized() ? now() - referenceEpoch : 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

inline void Clock::configure(int8_t offset)
{
    // Store the provided UTC offset
    utcOffset = offset;
}

// ─────────────────────────────────────────────────────────────────────────────
// Current Time
// ─────────────────────────────────────────────────────────────────────────────

inline uint32_t Clock::now() const
{
    if (!synchronized)
    {
        return 0;
    }

    // Extrapolate the current epoch time from the reference and the elapsed millis()
    return referenceEpoch +
        static_cast<uint32_t>((millis() - referenceMillis) / 1000);
}

// ─────────────────────────────────────────────────────────────────────────────
// Time Component Extraction
// ─────────────────────────────────────────────────────────────────────────────

inline uint8_t Clock::hour(uint32_t epoch) const
{
    // Apply the UTC offset before extracting the hour component
    return ((epoch + static_cast<int32_t>(utcOffset) * 3600) % 86400) / 3600;
}

inline uint8_t Clock::minute(uint32_t epoch) const
{
    return (epoch % 3600) / 60;
}

inline uint8_t Clock::second(uint32_t epoch) const
{
    return epoch % 60;
}

inline uint8_t Clock::day(uint32_t epoch) const
{
    // Convert the epoch time to a broken-down UTC calendar time
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_mday;
}

inline uint8_t Clock::month(uint32_t epoch) const
{
    // Convert the epoch time to a broken-down UTC calendar time
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_mon + 1;
}

inline uint16_t Clock::year(uint32_t epoch) const
{
    // Convert the epoch time to a broken-down UTC calendar time
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_year + 1900;
}