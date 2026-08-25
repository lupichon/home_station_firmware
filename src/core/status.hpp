/**
 * @file    status.hpp
 * @brief   Reports the success/failure state of the sensor reading and of
 *          each communication channel used to transmit data.
 * @author  Lucas Pichon
 * @date    2026-07-30
 */

#pragma once

#include <math.h>

// ============================================================
// Status struct definition
// ============================================================

/**
 * @brief Holds the last known success/failure state of the sensor reading
 *        and of each communication channel.
 */
struct Status
{
    bool sensorOK;      // Whether the sensor reading itself succeeded
    bool bluetoothOK;   // Whether the last transmission over Bluetooth succeeded
    bool lorawanOK;     // Whether the last transmission over LoRaWAN succeeded
};