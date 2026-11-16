/**
 * @file    helpers.hpp
 * @brief   Helper functions for the HomeStation firmware.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include <esp_system.h>

// Get a human-readable string representing the reason for the last reset of the ESP device
String getResetReasonString()
{
    switch (esp_reset_reason())
    {
        case ESP_RST_POWERON:   return "Power-on";
        case ESP_RST_EXT:       return "External pin";
        case ESP_RST_SW:        return "Software reset";
        case ESP_RST_PANIC:     return "Software panic";
        case ESP_RST_INT_WDT:   return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:  return "Task watchdog";
        case ESP_RST_WDT:       return "Other watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep sleep wakeup";
        case ESP_RST_BROWNOUT:  return "Brownout (low voltage)";
        case ESP_RST_SDIO:      return "SDIO reset";
        default:                return "Unknown";
    }
}