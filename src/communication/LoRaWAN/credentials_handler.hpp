#pragma once

#include <Arduino.h>
#include "../../core/storage.hpp"

// ============================================================
// Lecture d'une ligne hexadécimale sur le port série
// ============================================================

inline bool readHexLine(uint8_t* buffer, size_t expectedBytes)
{
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() != expectedBytes * 2)
    {
        Serial.print("Erreur : ");
        Serial.print(expectedBytes);
        Serial.println(" octets attendus (format hexadécimal, sans espaces).");
        return false;
    }

    for (size_t i = 0; i < expectedBytes; i++)
    {
        char byteStr[3] = { line[i * 2], line[i * 2 + 1], '\0' };
        buffer[i] = static_cast<uint8_t>(strtoul(byteStr, nullptr, 16));
    }

    return true;
}


// ============================================================
// Attend 3 secondes au démarrage : si l'utilisateur envoie "0",
// les credentials stockés sont effacés
// ============================================================

inline void checkResetCredentialsOnBoot(Storage& storage)
{
    Serial.println("Send 0 within the next 3 seconds to reset LoRaWAN credentials...");

    unsigned long start = millis();

    while (millis() - start < 3000)
    {
        String line = Serial.readStringUntil('\n');
        line.trim();

        if (line == "0")
        {
            storage.remove("devEui");
            storage.remove("appEui");
            storage.remove("appKey");

            Serial.println("LoRaWAN credentials erased.");
            return;
        }

        delay(10);
    }
}

inline void promptAndStore(Storage& storage, const char* key, const char* label, uint8_t* buffer, size_t byteCount)
{
    Serial.print(label);
    Serial.print(" not found in memory. Please enter it (");
    Serial.print(byteCount);
    Serial.println(" byte hexadecimal value - without spaces):");

    while (!readHexLine(buffer, byteCount))
    {
        // recommence tant que la saisie est invalide
    }
    Serial.print(label);
    for (size_t i = 0; i < byteCount; i++)
    {
        Serial.printf("%02X", buffer[i]);
    }
    Serial.println(" stored in memory.");

    storage.putBytes(key, buffer, byteCount);
}

inline void checkAndInitCredentials(Storage& storage, const char* key, const char* label, uint8_t* buffer, size_t byteCount)
{
    if (!storage.exists(key))
    {
        promptAndStore(storage, key, label, buffer, byteCount);
        storage.putBytes(key, buffer, byteCount);
    }
    storage.getBytes(key, buffer, byteCount);
}