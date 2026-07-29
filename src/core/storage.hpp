#pragma once

#include <Arduino.h>
#include <Preferences.h>


class Storage
{
    public:
        bool begin(const char* namespaceName, bool readOnly = false);
        void end();

        bool exists(const char* key);
        bool remove(const char* key);
        void clear();

        // ====================================================
        // Entiers
        // ====================================================

        bool putUInt(const char* key, uint32_t value);
        uint32_t getUInt(const char* key, uint32_t defaultValue = 0);

        // ====================================================
        // Booléens
        // ====================================================

        bool putBool(const char* key, bool value);
        bool getBool(const char* key, bool defaultValue = false);

        // ====================================================
        // Chaînes
        // ====================================================

        bool putString(const char* key, const String& value);
        String getString(const char* key, const String& defaultValue = "");

        // ====================================================
        // Données binaires
        // ====================================================

        bool putBytes(const char* key, const void* data, size_t size);
        size_t getBytes(const char* key, void* data, size_t size);
        size_t getBytesLength(const char* key);

    private:
        Preferences preferences;
};


// ============================================================
// Initialization
// ============================================================

inline bool Storage::begin(const char* namespaceName, bool readOnly)
{
    return preferences.begin(namespaceName, readOnly);
}


inline void Storage::end()
{
    preferences.end();
}


// ============================================================
// Gestion générique des clés
// ============================================================

inline bool Storage::exists(const char* key)
{
    return preferences.isKey(key);
}


inline bool Storage::remove(const char* key)
{
    return preferences.remove(key);
}


inline void Storage::clear()
{
    preferences.clear();
}


// ============================================================
// Entiers
// ============================================================

inline bool Storage::putUInt(const char* key, uint32_t value)
{
    return preferences.putUInt(key,value) > 0;
}


inline uint32_t Storage::getUInt(const char* key, uint32_t defaultValue)
{
    return preferences.getUInt(key,defaultValue);
}


// ============================================================
// Booléens
// ============================================================

inline bool Storage::putBool(const char* key, bool value)
{
    return preferences.putBool(key, value) > 0;
}


inline bool Storage::getBool(const char* key, bool defaultValue)
{
    return preferences.getBool(key,defaultValue);
}


// ============================================================
// Chaînes
// ============================================================

inline bool Storage::putString(const char* key, const String& value)
{
    return preferences.putString(key, value) > 0;
}


inline String Storage::getString(const char* key, const String& defaultValue)
{
    return preferences.getString(key, defaultValue);
}


// ============================================================
// Données binaires
// ============================================================

inline bool Storage::putBytes(const char* key, const void* data, size_t size)
{
    return preferences.putBytes(key, data, size) == size;
}


inline size_t Storage::getBytes(const char* key, void* data, size_t size)
{
    return preferences.getBytes(key, data, size);
}


inline size_t Storage::getBytesLength(const char* key)
{
    return preferences.getBytesLength(key);
}