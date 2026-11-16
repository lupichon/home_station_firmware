/**
 * @file    hex.hpp
 * @brief   Conversions between raw bytes and hexadecimal strings.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>

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
inline void hexToBytes(const char* hex, uint8_t* bytes, size_t len)
{
    // Convert a single hex character to its numeric value
    auto hexCharToVal = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };

    for (size_t i = 0; i < len; i++)
    {
        bytes[i] = (hexCharToVal(hex[i * 2]) << 4) | hexCharToVal(hex[i * 2 + 1]);
    }
}

/**
 * @brief Check whether a String is a valid hexadecimal string of a given length.
 * @param value          String to validate.
 * @param expectedLength Exact length the string must have to be considered valid.
 * @return true if value has exactly expectedLength characters and all of them
 *         are valid hex digits (0-9, a-f, A-F), false otherwise.
 */
inline bool isHexString(const String& value, size_t expectedLength)
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
