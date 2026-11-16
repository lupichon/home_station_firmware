/**
 * @file    sensors_bit.hpp
 * @brief   Bit identifiers of the sensors in the enabled-sensors mask.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stdint.h>

enum SensorsBit : uint16_t
{
    BH1750_BIT      = 1 << 0,  // Bit 0: BH1750 light sensor
    HCSR501_BIT     = 1 << 1,  // Bit 1: HC-SR501 motion sensor
    SCD41_BIT       = 1 << 2,  // Bit 2: SCD41 CO2 sensor
    MAX9814_BIT     = 1 << 3,  // Bit 3: MAX9814 microphone
    FC51_BIT        = 1 << 4,  // Bit 4: FC-51 obstacle sensor
    SW420_BIT       = 1 << 5,  // Bit 5: SW-420 vibration sensor
    MQ2_BIT         = 1 << 6,  // Bit 6: MQ-2 gas sensor
    BMP280_BIT      = 1 << 7,  // Bit 7: BMP280 pressure sensor
    SGP41_BIT       = 1 << 8,  // Bit 8: SGP41 air quality sensor
    REED_SWITCH_BIT = 1 << 9   // Bit 9: Reed switch sensor
};

inline bool isSensorEnabled(uint16_t enabledSensorsMask, SensorsBit sensor)
{
    return (enabledSensorsMask & sensor) != 0;
}
