/**
 * @file    communication.hpp
 * @brief   Base class for communication interfaces.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

// ============================================================
// Communication base class definition
// ============================================================

class Communication
{
    // ── Protected members ─────────────────────────────────────────────────
    protected:
        bool initialized = false;               // True if the communication interface has been successfully initialized
        static inline int  communicationCount;  // Number of Communication instances created

    // ── Public interface ──────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for Communication.
         */
        Communication();

        /**
         * @brief Initialize the communication interface.
         * @return true if initialization was successful, false otherwise.
         */
        virtual bool begin() = 0;

        /**
         * @brief Send data over the communication interface.
         * @param data Pointer to the data buffer.
         * @param size Size of the data to send in bytes.
         * @return true if data was sent successfully, false otherwise.
         */
        virtual bool send(uint8_t* data, size_t size) = 0;

        /**
         * @brief Check if the communication interface has been initialized.
         * @return true if initialized, false otherwise.
         */
        bool isInitialized() const;

        /**
         * @brief Get the total number of Communication instances created.
         * @return Number of Communication instances.
         */
        static int getCommunicationCount();

        /**
         * @brief Destructor for Communication.
         */
        virtual ~Communication() = default;
};

// ============================================================
// Implementations
// ============================================================

inline Communication::Communication()
    : initialized(false)
{
    communicationCount++;
}

inline bool Communication::isInitialized() const
{
    return initialized;
}

inline int Communication::getCommunicationCount()
{
    return communicationCount;
}