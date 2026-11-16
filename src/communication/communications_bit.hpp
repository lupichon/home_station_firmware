/**
 * @file    communications_bit.hpp
 * @brief   Bit identifiers of the communication interfaces in the enabled-comms mask.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <stdint.h>


enum CommsBit : uint8_t
{
    BLUETOOTH_BIT = 1 << 0, // Bit 0: Bluetooth
    LORAWAN_BIT   = 1 << 1, // Bit 1: LoRaWAN
    WIFI_BIT      = 1 << 2  // Bit 2: WiFi AP
};

inline bool isCommEnabled(uint8_t enabledCommsMask, CommsBit comm)
{
    return (enabledCommsMask & comm) != 0;
}