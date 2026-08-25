/**
 * @file    storage.hpp
 * @brief   Persistent storage interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-30
 */

#pragma once

#include <Arduino.h>
#include <Preferences.h>


// ============================================================
// Storage class definition
// ============================================================

class Storage
{
    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Open the given NVS namespace.
         * @param namespaceName Name of the NVS namespace to open.
         * @param readOnly      If true, open the namespace in read-only mode.
         * @return true if the namespace was opened successfully, false otherwise.
         */
        bool begin(const char* namespaceName, bool readOnly = false);

        /**
         * @brief Close the currently opened NVS namespace.
         */
        void end();

        /**
         * @brief Check whether a key exists in the current namespace.
         * @param key Key to look up.
         * @return true if the key exists, false otherwise.
         */
        bool exists(const char* key);

        /**
         * @brief Remove a key from the current namespace.
         * @param key Key to remove.
         * @return true if the key was removed successfully, false otherwise.
         */
        bool remove(const char* key);

        /**
         * @brief Erase all keys stored in the current namespace.
         */
        void clear();

        /**
         * @brief Store an unsigned 32-bit integer.
         * @param key   Key under which the value is stored.
         * @param value Value to store.
         * @return true if the value was stored successfully, false otherwise.
         */
        bool putUInt(const char* key, uint32_t value);

        /**
         * @brief Retrieve an unsigned 32-bit integer.
         * @param key          Key to read.
         * @param defaultValue Value returned if the key does not exist.
         * @return The stored value, or defaultValue if the key is absent.
         */
        uint32_t getUInt(const char* key, uint32_t defaultValue = 0);

        /**
         * @brief Store a boolean value.
         * @param key   Key under which the value is stored.
         * @param value Value to store.
         * @return true if the value was stored successfully, false otherwise.
         */
        bool putBool(const char* key, bool value);

        /**
         * @brief Retrieve a boolean value.
         * @param key          Key to read.
         * @param defaultValue Value returned if the key does not exist.
         * @return The stored value, or defaultValue if the key is absent.
         */
        bool getBool(const char* key, bool defaultValue = false);

        // ====================================================
        // Chaînes
        // ====================================================

        /**
         * @brief Store a string value.
         * @param key   Key under which the value is stored.
         * @param value Value to store.
         * @return true if the value was stored successfully, false otherwise.
         */
        bool putString(const char* key, const String& value);

        /**
         * @brief Retrieve a string value.
         * @param key          Key to read.
         * @param defaultValue Value returned if the key does not exist.
         * @return The stored value, or defaultValue if the key is absent.
         */
        String getString(const char* key, const String& defaultValue = "");

        /**
         * @brief Store a signed 8-bit integer.
         * @param key   Key under which the value is stored.
         * @param value Value to store.
         * @return true if the value was stored successfully, false otherwise.
         */
        bool putChar(const char* key, int8_t value);

        /**
         * @brief Retrieve a signed 8-bit integer.
         * @param key          Key to read.
         * @param defaultValue Value returned if the key does not exist.
         * @return The stored value, or defaultValue if the key is absent.
         */
        int8_t getChar(const char* key, int8_t defaultValue = 0);

        /**
         * @brief Store raw binary data.
         * @param key  Key under which the data is stored.
         * @param data Pointer to the data buffer to store.
         * @param size Size in bytes of the data buffer.
         * @return true if all bytes were stored successfully, false otherwise.
         */
        bool putBytes(const char* key, const void* data, size_t size);

        /**
         * @brief Retrieve raw binary data into a caller-provided buffer.
         * @param key  Key to read.
         * @param data Pointer to the destination buffer.
         * @param size Size in bytes of the destination buffer.
         * @return The number of bytes actually read.
         */
        size_t getBytes(const char* key, void* data, size_t size);

        /**
         * @brief Get the size in bytes of the data stored under a key.
         * @param key Key to inspect.
         * @return The size in bytes of the stored data, useful to size the
         *         destination buffer before calling getBytes().
         */
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

    // ── Private members ───────────────────────────────────────────────────
    private:
        Preferences preferences;
};


// ============================================================
// implementations of Storage methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool Storage::begin(const char* namespaceName, bool readOnly)
{
    return preferences.begin(namespaceName, readOnly);
}


inline void Storage::end()
{
    preferences.end();
}


// ─────────────────────────────────────────────────────────────────────────────
// Key Management
// ─────────────────────────────────────────────────────────────────────────────

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


// ─────────────────────────────────────────────────────────────────────────────
// Integer Storage
// ─────────────────────────────────────────────────────────────────────────────

inline bool Storage::putUInt(const char* key, uint32_t value)
{
    return preferences.putUInt(key,value) > 0;
}


inline uint32_t Storage::getUInt(const char* key, uint32_t defaultValue)
{
    return preferences.getUInt(key,defaultValue);
}


// ─────────────────────────────────────────────────────────────────────────────
// Boolean Storage
// ─────────────────────────────────────────────────────────────────────────────

inline bool Storage::putBool(const char* key, bool value)
{
    return preferences.putBool(key, value) > 0;
}


inline bool Storage::getBool(const char* key, bool defaultValue)
{
    return preferences.getBool(key,defaultValue);
}


// ─────────────────────────────────────────────────────────────────────────────
// String Storage
// ─────────────────────────────────────────────────────────────────────────────

inline bool Storage::putString(const char* key, const String& value)
{
    return preferences.putString(key, value) > 0;
}


inline String Storage::getString(const char* key, const String& defaultValue)
{
    return preferences.getString(key, defaultValue);
}

// ─────────────────────────────────────────────────────────────────────────────
// Signed Integer Storage
// ─────────────────────────────────────────────────────────────────────────────

inline bool Storage::putChar(const char* key, int8_t value)
{
    return preferences.putChar(key, value) > 0;
}

inline int8_t Storage::getChar(const char* key, int8_t defaultValue)
{
    return preferences.getChar(key, defaultValue);
}

// ─────────────────────────────────────────────────────────────────────────────
// Binary Data Storage
// ─────────────────────────────────────────────────────────────────────────────

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