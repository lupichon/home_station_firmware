#pragma once

#include <Arduino.h>

// ============================================================
// GPIO
// ============================================================
constexpr uint8_t PIN_GPIO_2  = 2;
constexpr uint8_t PIN_GPIO_4  = 4;  // Used
constexpr uint8_t PIN_GPIO_5  = 5;  // Used
constexpr uint8_t PIN_GPIO_12 = 12;
constexpr uint8_t PIN_GPIO_13 = 13;
constexpr uint8_t PIN_GPIO_14 = 14; // Used
constexpr uint8_t PIN_GPIO_15 = 15; // Used
constexpr uint8_t PIN_GPIO_16 = 16; // Used
constexpr uint8_t PIN_GPIO_17 = 17;
constexpr uint8_t PIN_GPIO_18 = 18; // Used
constexpr uint8_t PIN_GPIO_19 = 19; // Used
constexpr uint8_t PIN_GPIO_21 = 21; // Used
constexpr uint8_t PIN_GPIO_22 = 22; // Used
constexpr uint8_t PIN_GPIO_23 = 23; // Used
constexpr uint8_t PIN_GPIO_25 = 25; // Used
constexpr uint8_t PIN_GPIO_26 = 26; // Used
constexpr uint8_t PIN_GPIO_27 = 27; // Used
constexpr uint8_t PIN_GPIO_32 = 32; // Used
constexpr uint8_t PIN_GPIO_33 = 33; // Used
constexpr uint8_t PIN_GPIO_34 = 34; // Used
constexpr uint8_t PIN_GPIO_35 = 35; // Used
constexpr uint8_t PIN_GPIO_36 = 36; // Used
constexpr uint8_t PIN_GPIO_39 = 39;

// ============================================================
// Home Station
// ============================================================
constexpr uint8_t HC_SR501_PIN          = PIN_GPIO_34;
constexpr uint8_t MAX9814_PIN           = PIN_GPIO_35;
constexpr uint8_t FC_51_PIN             = PIN_GPIO_15;
constexpr uint8_t SW_420_PIN            = PIN_GPIO_33;
constexpr uint8_t MQ2_PIN               = PIN_GPIO_36;
constexpr uint8_t I2C_SDA_PIN           = PIN_GPIO_21;
constexpr uint8_t I2C_SCL_PIN           = PIN_GPIO_22;
constexpr uint8_t SPI_MISO_PIN          = PIN_GPIO_19;
constexpr uint8_t SPI_MOSI_PIN          = PIN_GPIO_23;
constexpr uint8_t SPI_SCK_PIN           = PIN_GPIO_18;
constexpr uint8_t SPI_NSS_PIN           = PIN_GPIO_5;
constexpr uint8_t SX1276_RST_PIN        = PIN_GPIO_14;
constexpr uint8_t SX1276_DIO0_PIN       = PIN_GPIO_4;
constexpr uint8_t SX1276_DIO1_PIN       = PIN_GPIO_16;
constexpr uint8_t STATUS_LED_RED_PIN    = PIN_GPIO_25;
constexpr uint8_t STATUS_LED_GREEN_PIN  = PIN_GPIO_26;
constexpr uint8_t STATUS_LED_BLUE_PIN   = PIN_GPIO_27;
constexpr uint8_t SCREEN_BUTTON_PIN     = PIN_GPIO_32;
constexpr uint8_t BUZZER_PIN            = PIN_GPIO_13;