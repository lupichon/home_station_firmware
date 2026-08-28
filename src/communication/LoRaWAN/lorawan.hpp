/**
 * @file    lorawan.hpp
 * @brief   LoRaWAN communication interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-29
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>
#include "../communication.hpp"

//TODO : A TESTER

extern "C"
{
    void os_getDevEui(u1_t* buf);
    void os_getArtEui(u1_t* buf);
    void os_getDevKey(u1_t* buf);
    void onEvent(ev_t event);
}

// ============================================================
// LoRaWANCommunication class definition
// ============================================================

class LoRaWANCommunication : public Communication
{
    public:
        using JoinedCallback        = void (*)();
        using DownlinkCallback      = void (*)(uint8_t port, const uint8_t* data, uint8_t length);
        using PayloadBuilder        = uint8_t (*)(uint8_t* buffer, uint8_t maxLength);
        using TxCompleteCallback    = void (*)(bool hasDownlink);
        using JoinFailedCallback    = void (*)();
        using RejoinFailedCallback  = void (*)();
        using LinkDeadCallback      = void (*)();
        using LinkAliveCallback     = void (*)();
        using RxStartCallback       = void (*)();
        using JoiningCallback       = void (*)();

        /**
         * @brief Constructor for LoRaWANCommunication.
         */
        LoRaWANCommunication();

        /**
         * @brief Configure the LoRaWAN communication parameters before initialization.
         * @param nss                 SPI Chip Select pin for the LoRa module.
         * @param rst                 Reset pin for the LoRa module.
         * @param dio0                DIO0 pin for the LoRa module.
         * @param dio1                DIO1 pin for the LoRa module.
         * @param devEui              Device EUI (8 bytes).
         * @param appEui              Application EUI (8 bytes).
         * @param appKey              Application Key (16 bytes).
         * @param payloadBuilder      Function pointer to a payload builder function that fills the buffer with data to send.
         * @param autoUplinkInterval  Interval in seconds for automatic uplink transmissions.
         */
        void configure(uint8_t nss, uint8_t rst, uint8_t dio0, uint8_t dio1, const uint8_t devEui[8], const uint8_t appEui[8], const uint8_t appKey[16], PayloadBuilder builder, uint32_t autoUplinkInterval);
        
        /**
         * @brief Initialize the LoRaWAN communication module.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Check if the LoRaWAN module is present on the SPI bus.
         * @return true if the module is detected, false otherwise.
         */
        bool checkModulePresence();

        /**
         * @brief Main loop to handle LoRaWAN events and automatic uplink transmissions.
         */
        void loop();

        /**
         * @brief Send an uplink message over LoRaWAN.
         * @param payload Pointer to the data buffer to send.
         * @param length Length of the data to send in bytes.
         * @param port LoRaWAN port number (default is 1).
         * @param confirmed true for confirmed uplink, false for unconfirmed (default is false).
         * @return true if the message was queued for transmission, false otherwise.
         */
        bool sendUplink(const uint8_t* payload, uint8_t length, uint8_t port = 1, bool confirmed = false);

        /**
         * @brief Check if the device has successfully joined the LoRaWAN network.
         * @return true if joined, false otherwise.
         */
        bool isJoined() const;

        /**
         * @brief Check if there is a pending transmission.
         * @return true if a transmission is pending, false otherwise.
         */
        bool isTxPending() const;
        
        void onJoined       (JoinedCallback       cb) { joinedCb        = cb; } // Callback for when the device successfully joins the LoRaWAN network
        void onDownlink     (DownlinkCallback     cb) { downlinkCb      = cb; } // Callback for when a downlink message is received
        void onTxComplete   (TxCompleteCallback   cb) { txCompleteCb    = cb; } // Callback for when a transmission is complete (with or without downlink)
        void onJoinFailed   (JoinFailedCallback   cb) { joinFailedCb    = cb; } // Callback for when the join process fails
        void onRejoinFailed (RejoinFailedCallback cb) { rejoinFailedCb  = cb; } // Callback for when a rejoin attempt fails
        void onLinkDead     (LinkDeadCallback     cb) { linkDeadCb      = cb; } // Callback for when the LoRaWAN link is considered dead (no downlink received for a while)
        void onLinkAlive    (LinkAliveCallback    cb) { linkAliveCb     = cb; } // Callback for when the LoRaWAN link is considered alive (downlink received after being dead)
        void onRxStart      (RxStartCallback      cb) { rxStartCb       = cb; } // Callback for when a downlink reception starts (DIO0 goes high)
        void onJoining      (JoiningCallback      cb) { joiningCb       = cb; } // Callback for when the device starts the join process

    private:
        uint8_t nssPin;     // SPI Chip Select pin for the LoRa module
        uint8_t rstPin;     // Reset pin for the LoRa module
        uint8_t dio0Pin;    // DIO0 pin for the LoRa module
        uint8_t dio1Pin;    // DIO1 pin for the LoRa module

        uint8_t devEui[8];   // Device EUI
        uint8_t appEui[8];   // Application EUI
        uint8_t appKey[16];  // Application Key

        bool joined = false;    // True if the device has successfully joined the LoRaWAN network

        uint32_t autoUplinkInterval = 60;        // Interval in seconds for automatic uplink transmissions
        PayloadBuilder payloadBuilder = nullptr; // Function to build the payload for uplink messages   


        // Callback function pointers for various LoRaWAN events
        JoinedCallback       joinedCb        = nullptr; 
        DownlinkCallback     downlinkCb      = nullptr;
        TxCompleteCallback   txCompleteCb    = nullptr;
        JoinFailedCallback   joinFailedCb    = nullptr;
        RejoinFailedCallback rejoinFailedCb  = nullptr;
        LinkDeadCallback     linkDeadCb      = nullptr;
        RxStartCallback      rxStartCb       = nullptr;
        JoiningCallback      joiningCb       = nullptr;
        LinkAliveCallback    linkAliveCb     = nullptr;

        osjob_t sendjob;    // Job structure for scheduling automatic uplink transmissions

        static constexpr uint8_t MAX_PAYLOAD_LEN = 51;  // Maximum payload length for LoRaWAN 

        /**
         * @brief Read a register from the LoRa module
         * @param address The address of the register to read
         * @return The value read from the register
         */
        uint8_t readRegister(uint8_t address);

        /**
         * @brief Handle a LoRaWAN event
         * @param event The event to handle
         */
        void handleEvent(ev_t event);

        /**
         * @brief Handle automatic uplink transmissions
         */
        void handleAutoSend();

        /**
         * @brief Relay function for scheduling automatic uplink transmissions
         * @param job The job structure
         */
        static void doSendRelay(osjob_t* job);

        // Friend functions to allow LMIC library to access private members of this class
        friend void os_getDevEui(u1_t* buf);
        friend void os_getArtEui(u1_t* buf);
        friend void os_getDevKey(u1_t* buf);
        friend void onEvent(ev_t event);
};

// ============================================================
// Implementation of LoRaWANCommunication methods
// ============================================================

extern LoRaWANCommunication* lorawanPtr;    // Pointer to the LoRaWANCommunication instance for LMIC callbacks

// ─────────────────────────────────────────────────────────────────────────────
// Credentials configuration
// ─────────────────────────────────────────────────────────────────────────────
extern "C"
{
    void os_getDevEui(u1_t* buf)
    {
        memcpy(buf, lorawanPtr->devEui, 8);
    }


    void os_getArtEui(u1_t* buf)
    {
        memcpy(buf, lorawanPtr->appEui, 8);
    }


    void os_getDevKey(u1_t* buf)
    {
        memcpy(buf, lorawanPtr->appKey, 16);
    }


    void onEvent(ev_t event)
    {
        lorawanPtr->handleEvent(event);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

inline LoRaWANCommunication::LoRaWANCommunication()
    : Communication()
{

}

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

inline void LoRaWANCommunication::configure(uint8_t nss, uint8_t rst, uint8_t dio0, uint8_t dio1, const uint8_t devEui[8], const uint8_t appEui[8], const uint8_t appKey[16], PayloadBuilder payloadBuilder, uint32_t autoUplinkInterval)
{
    nssPin = nss;
    rstPin = rst;
    dio0Pin = dio0;
    dio1Pin = dio1;

    memcpy(this->devEui, devEui, sizeof(this->devEui));
    memcpy(this->appEui, appEui, sizeof(this->appEui));
    memcpy(this->appKey, appKey, sizeof(this->appKey));
    this->payloadBuilder = payloadBuilder;
    this->autoUplinkInterval = autoUplinkInterval;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool LoRaWANCommunication::begin()
{
    pinMode(nssPin, OUTPUT);
    digitalWrite(nssPin, HIGH);

    if (!checkModulePresence())
    {
        return false;
    }

    lmic_pinmap pinmap =
    {
        .nss = nssPin,

        .rxtx = LMIC_UNUSED_PIN,

        .rst = rstPin,

        .dio =
        {
            dio0Pin,
            dio1Pin,
            LMIC_UNUSED_PIN
        }
    };

    os_init_ex(&pinmap);
    LMIC_reset();
    LMIC_setLinkCheckMode(0);
    LMIC_setAdrMode(1);

    LMIC_startJoining();

    initialized = true;
    return true;
}

inline uint8_t LoRaWANCommunication::readRegister(uint8_t address)
{
    digitalWrite(nssPin, LOW);
    SPI.transfer(address & 0x7F);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(nssPin, HIGH);

    return value;
}

inline bool LoRaWANCommunication::checkModulePresence()
{
    constexpr uint8_t REG_VERSION = 0x42;
    constexpr uint8_t EXPECTED_VERSION = 0x12;

    uint8_t version = readRegister(REG_VERSION);

    if (version == EXPECTED_VERSION)
    {
        return true;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────

inline void LoRaWANCommunication::loop()
{
    os_runloop_once();
}


// ─────────────────────────────────────────────────────────────────────────────
// Data Transmission
// ─────────────────────────────────────────────────────────────────────────────

inline bool LoRaWANCommunication::sendUplink(
    const uint8_t* payload,
    uint8_t length,
    uint8_t port,
    bool confirmed
)
{
    if (!joined || isTxPending())
    {
        return false;
    }

    LMIC_setTxData2(port, const_cast<uint8_t*>(payload), length, confirmed ? 1 : 0);

    return true;
}

inline void LoRaWANCommunication::handleAutoSend()
{
    if (!payloadBuilder || isTxPending())
    {
        return;
    }

    uint8_t buffer[MAX_PAYLOAD_LEN];
    uint8_t length = payloadBuilder(buffer, MAX_PAYLOAD_LEN);

    if (length == 0)
    {
        return;
    }

    LMIC_setTxData2(1, buffer, length, 0);
}

inline void LoRaWANCommunication::doSendRelay(osjob_t* job)
{
    lorawanPtr->handleAutoSend();
}

// ─────────────────────────────────────────────────────────────────────────────
// Event Handling
// ─────────────────────────────────────────────────────────────────────────────

inline void LoRaWANCommunication::handleEvent(ev_t event)
{
    switch (event)
    {
        case EV_JOINING:
            if (joiningCb) joiningCb();
            break;

        case EV_JOINED:
            joined = true;
            LMIC_setLinkCheckMode(0);
            LMIC_setAdrMode(1);
            if (joinedCb) joinedCb();
            handleAutoSend();
            break;

        case EV_JOIN_FAILED:
            joined = false;
            if (joinFailedCb) joinFailedCb();
            break;

        case EV_REJOIN_FAILED:
            joined = false;
            if (rejoinFailedCb) rejoinFailedCb();
            break;

        case EV_TXCOMPLETE:
            if (LMIC.dataLen && downlinkCb)
            {
                uint8_t port = LMIC.frame[LMIC.dataBeg - 1];
                downlinkCb(port, LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            }
            if (txCompleteCb) txCompleteCb(LMIC.dataLen > 0);
            if (payloadBuilder)
            {
                os_setTimedCallback(
                    &sendjob,
                    os_getTime() + sec2osticks(autoUplinkInterval),
                    doSendRelay
                );
            }
            break;

        case EV_LINK_DEAD:
            if (linkDeadCb) linkDeadCb();
            break;

        case EV_LINK_ALIVE:
            if (linkAliveCb) linkAliveCb();
            break;

        case EV_RXSTART:
            if (rxStartCb) rxStartCb();
            break;

        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Status Checks
// ─────────────────────────────────────────────────────────────────────────────

inline bool LoRaWANCommunication::isJoined() const
{
    return joined;
}

inline bool LoRaWANCommunication::isTxPending() const
{
    return (LMIC.opmode & OP_TXRXPEND) != 0;
}
