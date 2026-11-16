/**
 * @file    json_fields.hpp
 * @brief   Helpers to write single "key":value JSON fields into a String.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Build a float field with 2 decimals, or null if the value is NaN.
 */
inline String floatField(const char* key, float val)
{
    String s = "\"";
    s += key;
    s += "\":";
    s += isnan(val) ? "null" : String(val, 2);
    return s;
}

/**
 * @brief Build an unsigned integer field.
 */
inline String intField(const char* key, uint16_t val)
{
    String s = "\"";
    s += key;
    s += "\":";
    s += val;
    return s;
}

/**
 * @brief Build a boolean field written as 1 or 0.
 */
inline String boolField(const char* key, bool val)
{
    return "\"" + String(key) + "\":" + String(val ? 1 : 0);
}