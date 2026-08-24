#pragma once

#include <Arduino.h>
#include <functional>

#include "../core/clock.hpp"
#include "../interface/buzzer.hpp"

class AlarmManager
{
    public:

        using AlarmChangedCallback =
            std::function<void(bool armed, uint32_t targetEpoch)>;

        AlarmManager(Buzzer& buzzer, Clock& clock);
        void begin(bool initialArmed, uint32_t initialTargetEpoch);
        void update();
        void setAlarm(uint32_t targetEpoch);
        void dismiss();
        void onAlarmChanged(AlarmChangedCallback callback);
        bool isArmed() const;
        bool isRinging() const;
        uint32_t getTargetEpoch() const;

    private:
        Buzzer& buzzer;
        Clock& clock;
        bool ringing = false;

        AlarmChangedCallback alarmChangedCallback;

        bool armed = false;
        uint32_t targetEpoch = 0;
        uint32_t buzzStartTime = 0;

        static constexpr uint32_t LATE_TOLERANCE_S = 600;
        static constexpr uint32_t MAX_BUZZER_DURATION_MS = 60000;

        void clearAlarm();
        void notifyChanged();
};

inline AlarmManager::AlarmManager(Buzzer& buzzer, Clock& clock)
    : buzzer(buzzer), clock(clock)
{
}

inline void AlarmManager::begin(bool initialArmed, uint32_t initialTargetEpoch)
{
    armed = initialArmed;
    targetEpoch = initialTargetEpoch;
}

inline void AlarmManager::setAlarm(uint32_t newTargetEpoch)
{
    if (newTargetEpoch == 0)
    {
        buzzer.stop();
        return;
    }

    targetEpoch = newTargetEpoch;
    armed = true;
    notifyChanged();
}

inline void AlarmManager::dismiss()
{
    buzzer.stop();
    clearAlarm();
}

inline void AlarmManager::update()
{
    if (!armed && buzzer.isActive())
    {
        if (millis() - buzzStartTime >= MAX_BUZZER_DURATION_MS)
        {
            buzzer.stop();
            ringing = false;
        }
    }
    if (!armed || !clock.isSynchronized())
    {
        return;
    }

    uint32_t now = clock.now();

    if (now < targetEpoch)
    {
        return;
    }

    uint32_t lateBy = now - targetEpoch;

    if (lateBy <= LATE_TOLERANCE_S)
    {
        buzzer.start();
        buzzStartTime = millis();
        ringing = true;
    }

    clearAlarm();
}

inline void AlarmManager::clearAlarm()
{
    armed = false;
    ringing = false;
    targetEpoch = 0;
    notifyChanged();
}

inline void AlarmManager::notifyChanged()
{
    if (alarmChangedCallback)
    {
        alarmChangedCallback(armed, targetEpoch);
    }
}

inline void AlarmManager::onAlarmChanged(AlarmChangedCallback callback)
{
    alarmChangedCallback = callback;
}

inline bool AlarmManager::isArmed() const
{
    return armed;
}   

inline uint32_t AlarmManager::getTargetEpoch() const
{
    return targetEpoch;
}

inline bool AlarmManager::isRinging() const
{
    return ringing;
}