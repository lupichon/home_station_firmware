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

        LoRaWANCommunication() = default;
        void configure(uint8_t nss, uint8_t rst, uint8_t dio0, uint8_t dio1, const uint8_t devEui[8], const uint8_t appEui[8], const uint8_t appKey[16], PayloadBuilder builder, uint32_t autoUplinkInterval);
        bool begin() override;
        bool checkModulePresence();
        void loop();
        bool sendUplink(const uint8_t* payload, uint8_t length, uint8_t port = 1, bool confirmed = false);
        bool isJoined() const;
        bool isTxPending() const;
        void setAutoUplinkInterval(uint32_t seconds);
        bool send(uint8_t* data, size_t size) override;
        
        void onJoined       (JoinedCallback       cb) { joinedCb        = cb; }
        void onDownlink     (DownlinkCallback     cb) { downlinkCb      = cb; }
        void onTxComplete   (TxCompleteCallback   cb) { txCompleteCb    = cb; }
        void onJoinFailed   (JoinFailedCallback   cb) { joinFailedCb    = cb; }
        void onRejoinFailed (RejoinFailedCallback cb) { rejoinFailedCb  = cb; }
        void onLinkDead     (LinkDeadCallback     cb) { linkDeadCb      = cb; }
        void onLinkAlive    (LinkAliveCallback    cb) { linkAliveCb     = cb; }
        void onRxStart      (RxStartCallback      cb) { rxStartCb       = cb; }
        void onJoining      (JoiningCallback      cb) { joiningCb       = cb; }

    private:
        uint8_t nssPin;
        uint8_t rstPin;
        uint8_t dio0Pin;
        uint8_t dio1Pin;

        uint8_t devEui[8];
        uint8_t appEui[8];
        uint8_t appKey[16];

        bool joined = false;

        uint32_t autoUplinkInterval = 60;
        PayloadBuilder payloadBuilder = nullptr;


        JoinedCallback       joinedCb        = nullptr;
        DownlinkCallback     downlinkCb      = nullptr;
        TxCompleteCallback   txCompleteCb    = nullptr;
        JoinFailedCallback   joinFailedCb    = nullptr;
        RejoinFailedCallback rejoinFailedCb  = nullptr;
        LinkDeadCallback     linkDeadCb      = nullptr;
        RxStartCallback      rxStartCb       = nullptr;
        JoiningCallback      joiningCb       = nullptr;
        LinkAliveCallback    linkAliveCb     = nullptr;

        osjob_t sendjob;

        static constexpr uint8_t MAX_PAYLOAD_LEN = 51;

        uint8_t readRegister(uint8_t address);
        void handleEvent(ev_t event);
        void handleAutoSend();
        static void doSendRelay(osjob_t* job);
        friend void os_getDevEui(u1_t* buf);
        friend void os_getArtEui(u1_t* buf);
        friend void os_getDevKey(u1_t* buf);
        friend void onEvent(ev_t event);
};


extern LoRaWANCommunication* lorawanPtr;

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

// ============================================================
// Constructeur
// ============================================================

inline LoRaWANCommunication::LoRaWANCommunication()
    : Communication()
{

}

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


inline uint8_t LoRaWANCommunication::readRegister(uint8_t address)
{
    digitalWrite(nssPin, LOW);
    SPI.transfer(address & 0x7F);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(nssPin, HIGH);

    return value;
}


// ============================================================
// Vérification de présence du SX1276
// ============================================================

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


// ============================================================
// begin()
// ============================================================

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


// ============================================================
// loop()
// ============================================================

inline void LoRaWANCommunication::loop()
{
    os_runloop_once();
}


// ============================================================
// Envoi manuel
// ============================================================

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


// ============================================================
// Vérification Join
// ============================================================

inline bool LoRaWANCommunication::isJoined() const
{
    return joined;
}


// ============================================================
// Vérification transmission en cours
// ============================================================

inline bool LoRaWANCommunication::isTxPending() const
{
    return (LMIC.opmode & OP_TXRXPEND) != 0;
}


// ============================================================
// Configuration intervalle uplink automatique
// ============================================================

inline void LoRaWANCommunication::setAutoUplinkInterval(uint32_t seconds)
{
    autoUplinkInterval = seconds;
}


// ============================================================
// Envoi automatique
// ============================================================

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

// ============================================================
// Relais envoi automatique
// ============================================================

inline void LoRaWANCommunication::doSendRelay(osjob_t* job)
{
    lorawanPtr->handleAutoSend();
}


// ============================================================
// Gestion événements LMIC
// ============================================================

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

inline bool LoRaWANCommunication::send(uint8_t* data, size_t size)
{
    if (size > MAX_PAYLOAD_LEN)
    {
        return false;
    }

    return sendUplink(data, static_cast<uint8_t>(size));
}