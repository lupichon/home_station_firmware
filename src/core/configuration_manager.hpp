/**
 * @file    serial_config_utils.hpp
 * @brief   Serial-based helpers to read/validate user input and to load,
 *          create, and reset persisted device configuration values (binary,
 *          string, hex) via Storage.
 * @author  Lucas Pichon
 * @date    2026-08-20
 */

#pragma once

#include <Arduino.h>
#include "storage.hpp"

// ============================================================
// Lecture d'une valeur hexadécimale
// ============================================================

/**
 * @brief Block until a line is available on Serial and parse it as a hexadecimal
 *        string into a raw byte buffer.
 * @param buffer        Destination buffer to write the decoded bytes into.
 * @param expectedBytes Number of bytes expected (the input line must contain
 *                       exactly expectedBytes * 2 hex characters).
 * @return true if a valid line was read and decoded, false if the line had
 *         the wrong length (in which case buffer is left untouched).
 */
inline bool readHexLine(uint8_t* buffer, size_t expectedBytes)
{
    // Wait for a line to be available on the serial port
    while (!Serial.available()) { delay(10); }

    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() != expectedBytes * 2)
    {
        // Wrong number of hex characters for the requested byte count
        Serial.print("Error: ");
        Serial.print(expectedBytes);
        Serial.println(" bytes expected.");
        return false;
    }

    // Decode the line two characters at a time into raw bytes
    for (size_t i = 0; i < expectedBytes; i++)
    {
        char byteStr[3] =
        {
            line[i * 2],
            line[i * 2 + 1],
            '\0'
        };

        buffer[i] = static_cast<uint8_t>(
            strtoul(byteStr, nullptr, 16)
        );
    }

    return true;
}


// ============================================================
// Lecture d'une chaîne
// ============================================================

/**
 * @brief Block until a line is available on Serial and copy it into a char buffer,
 *        validating its length against the buffer size.
 * @param buffer    Destination buffer to write the string into (null-terminated).
 * @param size      Total size of the destination buffer (including the null terminator).
 * @param exactSize If true, the input line must have exactly size - 1 characters.
 *                  If false, the input line must have between 1 and size - 1 characters.
 * @return true if a valid line was read and copied, false if the line length
 *         didn't satisfy the exactSize constraint (in which case buffer is left untouched).
 */
inline bool readStringLine(char* buffer, size_t size, bool exactSize = true)
{
    // Wait for a line to be available on the serial port
    while (!Serial.available()) { delay(10); }

    String line = Serial.readStringUntil('\n');
    line.trim();

    bool valid = exactSize
        ? line.length() == size - 1
        : line.length() > 0 && line.length() <= size - 1;

    if (!valid)
    {
        // Line length doesn't satisfy the exact/range constraint
        Serial.print("Error: ");
        if (exactSize)
        {
            Serial.print(size - 1);
            Serial.println(" characters expected.");
        }
        else
        {
            Serial.print("1 to ");
            Serial.print(size - 1);
            Serial.println(" characters expected.");
        }
        return false;
    }

    line.toCharArray(buffer, size);

    return true;
}


// ============================================================
// Stockage configuration binaire
// ============================================================

/**
 * @brief Load a binary configuration value from storage, prompting the user
 *        over Serial to enter it (as a hex string) if it doesn't exist yet.
 * @param storage Storage instance to read from / write to.
 * @param key     Storage key under which the value is/will be stored.
 * @param label   Human-readable label used in the Serial prompt/messages.
 * @param buffer  Buffer used both to receive user input and to hold the loaded value.
 * @param size    Size in bytes of the value.
 */
inline void loadOrCreateConfig(
    Storage& storage,
    const char* key,
    const char* label,
    uint8_t* buffer,
    size_t size
)
{
    if (!storage.exists(key))
    {
        // Value not yet stored: prompt the user to enter it over Serial
        Serial.print(label);
        Serial.println(" not found.");
        Serial.print("Please enter a value of ");
        Serial.print(size - 1);
        Serial.println(" characters:");

        while(!readHexLine(buffer, size))
        {
            // Keep retrying until a valid hex line is entered
        }

        storage.putBytes(key, buffer, size);
    }

    // Load the (now guaranteed to exist) value into the buffer
    storage.getBytes(key, buffer, size);
}


// ============================================================
// Stockage configuration texte
// ============================================================

/**
 * @brief Load a string configuration value from storage, prompting the user
 *        over Serial to enter it if it doesn't exist yet.
 * @param storage   Storage instance to read from / write to.
 * @param key       Storage key under which the value is/will be stored.
 * @param label     Human-readable label used in the Serial prompt/messages.
 * @param value     Output String that will receive the loaded value.
 * @param maxSize   Maximum number of characters allowed for the value.
 * @param exactSize If true, the entered value must be exactly maxSize characters.
 *                  If false, it must be between 1 and maxSize characters.
 */
inline void loadOrCreateConfig(
    Storage& storage,
    const char* key,
    const char* label,
    String& value,
    size_t maxSize,
    bool exactSize = true
)
{
    if (!storage.exists(key))
    {
        // Value not yet stored: prompt the user to enter it over Serial
        Serial.print(label);
        Serial.println(" not found.");

        if (exactSize)
        {
            Serial.print("Please enter a value of ");
            Serial.print(maxSize);
            Serial.println(" characters:");
        }
        else
        {
            Serial.print("Please enter a value of 1 to ");
            Serial.print(maxSize);
            Serial.println(" characters:");
        }

        char buf[maxSize + 1];
        while (!readStringLine(buf, maxSize + 1, exactSize))
        {
            // Keep retrying until a valid line is entered
        }
        
        storage.putString(key, buf);
    }

    // Load the (now guaranteed to exist) value
    value = storage.getString(key);
}


// ============================================================
// Reset au boot
// ============================================================

/**
 * @brief Give the user a short window at boot to reset parts (or all) of the
 *        stored configuration by sending a command over Serial.
 *        Waits up to 3 seconds for a command; does nothing if none is received.
 * @param storage Storage instance to remove/clear keys from.
 *
 * Commands:
 * - "0": reset LoRaWAN configuration (devEUI, appEUI, appKey)
 * - "1": reset Bluetooth configuration (BLE name, service UUID, characteristic UUID)
 * - "2": reset WiFi AP configuration (SSID, password)
 * - "3": reset everything (clears all storage) and return immediately
 */
inline void checkConfigResetOnBoot(Storage& storage)
{
    Serial.println();
    Serial.println("Send:");
    Serial.println("0 -> Reset LoRaWAN");
    Serial.println("1 -> Reset Bluetooth");
    Serial.println("2 -> Reset WiFi AP");
    Serial.println("3 -> Reset everything");

    unsigned long start = millis();

    // Wait up to 3 seconds for a command to be entered
    while(millis() - start < 3000)
    {
        if(Serial.available())
        {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();

            if(cmd == "0")
            {
                // Reset LoRaWAN-related keys
                storage.remove(Storage::devEUIKey);
                storage.remove(Storage::appEUIKey);
                storage.remove(Storage::appKeyKey);

                Serial.println("LoRaWAN reset.");
            }

            else if(cmd == "1")
            {
                // Reset Bluetooth-related keys
                storage.remove(Storage::bleNameKey);
                storage.remove(Storage::serviceUUIDKey);
                storage.remove(Storage::characteristicUUIDKey);

                Serial.println("Bluetooth reset.");
            }

            else if(cmd == "2")
            {
                // Reset WiFi AP-related keys
                storage.remove(Storage::wifiApSSIDKey);
                storage.remove(Storage::wifiApPasswordKey);

                Serial.println("WiFi AP reset.");
            }

            else if(cmd == "3")
            {
                // Full reset: clear all storage and exit immediately
                storage.clear();

                Serial.println("Full reset.");
                return;
            }

            else 
            {
                Serial.println("Invalid command.");
            }
        }

        delay(10);
    }
}

// ============================================================
// Conversion hexadécimale
// ============================================================

/**
 * @brief Convert a raw byte buffer into its uppercase hexadecimal string representation.
 * @param data Pointer to the byte buffer to convert.
 * @param size Number of bytes to convert.
 * @return An uppercase hex string of length size * 2 (e.g. {0xAB, 0x0F} -> "AB0F").
 */
inline String bytesToHex(const uint8_t* data, size_t size)
{
    String result;

    for (size_t i = 0; i < size; i++)
    {
        // Pad single-digit hex values with a leading zero
        if (data[i] < 0x10)
            result += "0";

        result += String(data[i], HEX);
    }

    result.toUpperCase();

    return result;
}

/**
 * @brief Convert a hexadecimal C-string into a raw byte buffer.
 * @param hex   Null-terminated hex string to convert (must contain at least len * 2 characters).
 * @param bytes Destination buffer to write the decoded bytes into.
 * @param len   Number of bytes to decode (i.e. hex string is read two characters at a time).
 */
static inline void hexToBytes(const char* hex, uint8_t* bytes, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        char hi = hex[i * 2];
        char lo = hex[i * 2 + 1];

        // Convert a single hex character to its numeric value
        auto hexCharToVal = [](char c) -> uint8_t {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return 0;
        };

        bytes[i] = (hexCharToVal(hi) << 4) | hexCharToVal(lo);
    }
}

/**
 * @brief Check whether a String is a valid hexadecimal string of a given length.
 * @param value          String to validate.
 * @param expectedLength Exact length the string must have to be considered valid.
 * @return true if value has exactly expectedLength characters and all of them
 *         are valid hex digits (0-9, a-f, A-F), false otherwise.
 */
inline bool isHexString(const String value, size_t expectedLength)
{
    if (value.length() != expectedLength) return false;
    
    for (size_t i = 0; i < value.length(); i++)
    {
        char c = value[i];
        bool isDigit = (c >= '0' && c <= '9');
        bool isLower = (c >= 'a' && c <= 'f');
        bool isUpper = (c >= 'A' && c <= 'F');
        if (!isDigit && !isLower && !isUpper) return false;
    }
    return true;
}