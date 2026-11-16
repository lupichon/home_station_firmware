/**
 * @file    pins.hpp
 * @brief   Defines the raw ESP32 GPIO pin numbers and their assignment to
 *          the Home Station's sensors, peripherals, and communication buses.
 * @author  Lucas Pichon
 * @date    2026-07-23
 */

#pragma once

#include <Arduino.h>

// ============================================================
// GPIO
// ============================================================

// Raw ESP32 GPIO pin numbers. "// Used" marks pins currently assigned
// to a sensor/peripheral/bus below; unmarked pins are available/unused.
constexpr uint8_t PIN_GPIO_2  = 2;
constexpr uint8_t PIN_GPIO_4  = 4;  // Used
constexpr uint8_t PIN_GPIO_5  = 5;  // Used
constexpr uint8_t PIN_GPIO_12 = 12;
constexpr uint8_t PIN_GPIO_13 = 13; // Used
constexpr uint8_t PIN_GPIO_14 = 14; // Used
constexpr uint8_t PIN_GPIO_15 = 15; // Used
constexpr uint8_t PIN_GPIO_16 = 16; // Used
constexpr uint8_t PIN_GPIO_17 = 17; // Used
constexpr uint8_t PIN_GPIO_18 = 18; // Used
constexpr uint8_t PIN_GPIO_19 = 19; // Used
constexpr uint8_t PIN_GPIO_21 = 21; // Used
constexpr uint8_t PIN_GPIO_22 = 22; // Used
constexpr uint8_t PIN_GPIO_23 = 23; // Used
constexpr uint8_t PIN_GPIO_25 = 25; // Used
constexpr uint8_t PIN_GPIO_26 = 26; // Used
constexpr uint8_t PIN_GPIO_27 = 27; // Used
constexpr uint8_t PIN_GPIO_32 = 32; // Used
constexpr uint8_t PIN_GPIO_33 = 33; 
constexpr uint8_t PIN_GPIO_34 = 34; // Used
constexpr uint8_t PIN_GPIO_35 = 35; // Used
constexpr uint8_t PIN_GPIO_36 = 36; // Used
constexpr uint8_t PIN_GPIO_39 = 39;

// ============================================================
// Home Station
// ============================================================

// Sensor, peripheral, and bus pin assignments, mapped to the raw GPIO
// numbers above. Update here (not the sensor code) if the wiring changes.
constexpr uint8_t HC_SR501_PIN          = PIN_GPIO_34;   // PIR motion sensor
constexpr uint8_t MAX9814_PIN           = PIN_GPIO_35;   // Microphone amplifier (sound detection)
constexpr uint8_t FC_51_PIN             = PIN_GPIO_15;   // IR obstacle/proximity sensor
constexpr uint8_t SW_420_PIN            = PIN_GPIO_17;   // Vibration sensor
constexpr uint8_t MQ2_PIN               = PIN_GPIO_36;   // Gas sensor (analog)
constexpr uint8_t I2C_SDA_PIN           = PIN_GPIO_21;   // I2C data line (shared bus)
constexpr uint8_t I2C_SCL_PIN           = PIN_GPIO_22;   // I2C clock line (shared bus)
constexpr uint8_t SPI_MISO_PIN          = PIN_GPIO_19;   // SPI MISO (shared bus)
constexpr uint8_t SPI_MOSI_PIN          = PIN_GPIO_23;   // SPI MOSI (shared bus)
constexpr uint8_t SPI_SCK_PIN           = PIN_GPIO_18;   // SPI clock (shared bus)
constexpr uint8_t SPI_NSS_PIN           = PIN_GPIO_5;    // SPI chip select for the SX1276 (LoRa)
constexpr uint8_t SX1276_RST_PIN        = PIN_GPIO_14;   // SX1276 (LoRa) reset pin
constexpr uint8_t SX1276_DIO0_PIN       = PIN_GPIO_4;    // SX1276 (LoRa) DIO0 interrupt pin
constexpr uint8_t SX1276_DIO1_PIN       = PIN_GPIO_16;   // SX1276 (LoRa) DIO1 interrupt pin
constexpr uint8_t STATUS_LED_RED_PIN    = PIN_GPIO_25;   // Status RGB LED - red channel
constexpr uint8_t STATUS_LED_GREEN_PIN  = PIN_GPIO_26;   // Status RGB LED - green channel
constexpr uint8_t STATUS_LED_BLUE_PIN   = PIN_GPIO_27;   // Status RGB LED - blue channel
constexpr uint8_t SCREEN_BUTTON_PIN     = PIN_GPIO_32;   // Push button for screen navigation
constexpr uint8_t BUZZER_PIN            = PIN_GPIO_13;   // Buzzer (alarm)