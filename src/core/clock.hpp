#pragma once

#include <Arduino.h>
#include <time.h>

class Clock
{
    public:
        void sync(uint32_t epoch);
        
        bool isSynchronized() const;
        void setUtcOffset(int8_t offset);

        uint32_t now() const;

        uint8_t hour(uint32_t epoch) const;
        uint8_t minute(uint32_t epoch) const;
        uint8_t second(uint32_t epoch) const;
        uint8_t day(uint32_t epoch) const;
        uint8_t month(uint32_t epoch) const;
        uint16_t year(uint32_t epoch) const;

    private:
        bool synchronized = false;

        uint32_t referenceEpoch = 0;
        uint32_t referenceMillis = 0;

        int8_t utcOffset = 0;
};


inline void Clock::sync(uint32_t epoch)
{
    referenceEpoch = epoch;
    referenceMillis = millis();
    synchronized = true;
}


inline bool Clock::isSynchronized() const
{
    return synchronized;
}


inline uint32_t Clock::now() const
{
    if (!synchronized)
    {
        return 0;
    }

    return referenceEpoch +
           static_cast<uint32_t>((millis() - referenceMillis) / 1000);
}


inline uint8_t Clock::hour(uint32_t epoch) const
{
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
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_mday;
}

inline uint8_t Clock::month(uint32_t epoch) const
{
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_mon + 1;
}

inline uint16_t Clock::year(uint32_t epoch) const
{
    time_t t = epoch;
    struct tm* timeinfo = gmtime(&t);
    return timeinfo->tm_year + 1900;
}

inline void Clock::setUtcOffset(int8_t offset)
{
    utcOffset = offset;
}