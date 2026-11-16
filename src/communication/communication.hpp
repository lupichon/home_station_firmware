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
        bool initialized = false;   // True if the communication interface has been successfully initialized

    // ── Public interface ──────────────────────────────────────────────────
    public:
        virtual ~Communication() = default;

        /**
         * @brief Initialize the communication interface.
         * @return true if initialization was successful, false otherwise.
         */
        virtual bool begin() = 0;

        /**
         * @brief Run the periodic work of the interface. Does nothing by default.
         */
        virtual void loop() {}

        /**
         * @brief Stop the interface. Does nothing by default.
         */
        virtual void stop() {}

        /**
         * @brief Send data over the interface. Fails by default, for interfaces that do not send.
         * @param data Pointer to the data buffer.
         * @param size Size of the data to send in bytes.
         * @return true if data was sent successfully, false otherwise.
         */
        virtual bool send(uint8_t* data, size_t size) { return false; }

        /**
         * @brief Check if a peer is connected. False by default.
         */
        virtual bool hasConnectedClient() const { return false; }

        /**
         * @brief Check if the communication interface has been initialized.
         * @return true if initialized, false otherwise.
         */
        bool isInitialized() const { return initialized; }
};
