#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CSgp41.h>
#include <NOxGasIndexAlgorithm.h>
#include <VOCGasIndexAlgorithm.h>
#include "../sensor.hpp"

//TODO : A TESTER

class SGP41Sensor : public Sensor
{
    private:
        SensirionI2CSgp41 sensor;
        VOCGasIndexAlgorithm vocAlgorithm;
        NOxGasIndexAlgorithm noxAlgorithm;
        unsigned long lastReadTime = 0;
        static constexpr unsigned long READ_INTERVAL_MS = 1000;
        uint16_t lastVocIndex = NAN;
        uint16_t lastNoxIndex = NAN;

    public:
        SGP41Sensor();
        bool begin() override;
        bool read(Measurement& m) override;
        const char* displayValue(const Measurement& m) const;
};

inline SGP41Sensor::SGP41Sensor()
    : Sensor("SGP41")
{
}

inline bool SGP41Sensor::begin()
{
    sensor.begin(Wire);

    uint16_t error;
    uint16_t srawVoc;

    for (uint16_t i = 0; i < 10; i++)
    {
        error = sensor.executeConditioning(0x8000, 0x6666, srawVoc);
        delay(1000);
    }

    initialized = (error == 0);
    return initialized;
}

inline bool SGP41Sensor::read(Measurement& m)
{
    if (!initialized) return false;

    unsigned long now = millis();
    if (now - lastReadTime < READ_INTERVAL_MS) 
    {
        m.vocIndex = lastVocIndex;
        m.noxIndex = lastNoxIndex;
        return true;
    }
    lastReadTime = now;

    uint16_t humComp = 0x8000; // Default relative humidity (50%)
    uint16_t tempComp = 0x6666; // Default temperature (25°C)
    if (!isnan(m.humidity))
    {
        humComp  = (uint16_t)((m.humidity    / 100.0) * 65535.0);
    }
    if (!isnan(m.temperature))
    {
        tempComp = (uint16_t)(((m.temperature + 45.0) / 175.0) * 65535.0);
    }

    uint16_t srawVoc;
    uint16_t srawNox;

    uint16_t error = sensor.measureRawSignals(humComp, tempComp, srawVoc, srawNox);

    if (error != 0) return false;

    lastVocIndex = m.vocIndex = (uint16_t) vocAlgorithm.process(srawVoc);
    lastNoxIndex = m.noxIndex = (uint16_t) noxAlgorithm.process(srawNox);

    return true;
}

inline const char* SGP41Sensor::displayValue(const Measurement& m) const
{
    static char buffer[64];
    snprintf(buffer, sizeof(buffer),
        "%s: VOC=%u NOx=%u",
        getName(), m.vocIndex, m.noxIndex);
    return buffer;
}