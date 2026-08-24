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
        // Entiers signés
        // ====================================================
        bool putChar(const char* key, int8_t value);
        int8_t getChar(const char* key, int8_t defaultValue = 0);

        // ====================================================
        // Données binaires
        // ====================================================

        bool putBytes(const char* key, const void* data, size_t size);
        size_t getBytes(const char* key, void* data, size_t size);
        size_t getBytesLength(const char* key);

        static constexpr const char* STORAGE_NAMESPACE = "HomeStation";
        static constexpr const char* devEUIKey = "devEui";
        static constexpr const char* appEUIKey = "appEui";
        static constexpr const char* appKeyKey = "appKey";
        static constexpr const char* bleNameKey = "bleName";
        static constexpr const char* serviceUUIDKey = "serUUID";
        static constexpr const char* characteristicUUIDKey = "charUUID";
        static constexpr const char* timeSyncUUIDKey = "tSynUUID";
        static constexpr const char* alarmTargetUUIDKey = "alTaUUID";
        static constexpr const char* wifiApSSIDKey = "apSSID";
        static constexpr const char* wifiApPasswordKey = "apPass";
        static constexpr const char* utcOffsetKey = "utcOffset";
        static constexpr const char* alarmArmedKey = "alarmArmed";
        static constexpr const char* alarmTargetKey = "alarmTarget";
        static constexpr const char* enabledSensorsMaskKey = "enSensMask";

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
// Entiers signés
// ============================================================
inline bool Storage::putChar(const char* key, int8_t value)
{
    return preferences.putChar(key, value) > 0;
}

inline int8_t Storage::getChar(const char* key, int8_t defaultValue)
{
    return preferences.getChar(key, defaultValue);
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
