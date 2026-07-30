#pragma once

#include <Arduino.h>
#include "storage.hpp"

// ============================================================
// Lecture d'une valeur hexadécimale
// ============================================================

inline bool readHexLine(uint8_t* buffer, size_t expectedBytes)
{
    while (!Serial.available()) { delay(10); }

    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() != expectedBytes * 2)
    {
        Serial.print("Error: ");
        Serial.print(expectedBytes);
        Serial.println(" bytes expected.");
        return false;
    }

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

inline bool readStringLine(char* buffer, size_t size, bool exactSize = true)
{
    while (!Serial.available()) { delay(10); }

    String line = Serial.readStringUntil('\n');
    line.trim();

    bool valid = exactSize
        ? line.length() == size - 1
        : line.length() > 0 && line.length() <= size - 1;

    if (!valid)
    {
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
        Serial.print(label);
        Serial.println(" not found.");
        Serial.print("Please enter a value of ");
        Serial.print(size - 1);
        Serial.println(" characters:");

        while(!readHexLine(buffer, size))
        {
        }

        storage.putBytes(key, buffer, size);
    }

    storage.getBytes(key, buffer, size);
}


// ============================================================
// Stockage configuration texte
// ============================================================

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

        }
        
        storage.putString(key, buf);
    }

    value = storage.getString(key);
}

inline void checkConfigResetOnBoot(Storage& storage)
{
    Serial.println();
    Serial.println("Send:");
    Serial.println("0 -> Reset LoRaWAN");
    Serial.println("1 -> Reset Bluetooth");
    Serial.println("2 -> Reset everything");


    unsigned long start = millis();

    while(millis() - start < 3000)
    {
        if(Serial.available())
        {
            String cmd = Serial.readStringUntil('\n');
            cmd.trim();


            if(cmd == "0")
            {
                storage.remove("devEui");
                storage.remove("appEui");
                storage.remove("appKey");

                Serial.println("LoRaWAN reset.");
                return;
            }


            if(cmd == "1")
            {
                storage.remove("bleName");
                storage.remove("serUUID");
                storage.remove("charUUID");

                Serial.println("Bluetooth reset.");
                return;
            }


            if(cmd == "2")
            {
                storage.clear();

                Serial.println("Full reset.");
                return;
            }
        }

        delay(10);
    }
}