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
        struct RadioPins
        {
            uint8_t nss;
            uint8_t rst;

            uint8_t dio0;
            uint8_t dio1;
        };


        using JoinedCallback = void (*)();


        using DownlinkCallback =
            void (*)
            (
                uint8_t port,
                const uint8_t* data,
                uint8_t length
            );


        using PayloadBuilder =
            uint8_t (*)
            (
                uint8_t* buffer,
                uint8_t maxLength
            );


        LoRaWANCommunication(const RadioPins& pins, const uint8_t devEui[8], const uint8_t appEui[8], const uint8_t appKey[16]);
        LoRaWANCommunication() = default;
        bool begin() override;
        void loop();
        bool checkModulePresence();
        bool sendUplink(const uint8_t* payload, uint8_t length, uint8_t port = 1, bool confirmed = false);
        bool isJoined() const;
        bool isTxPending() const;
        void setAutoUplinkInterval(uint32_t seconds);
        void setPayloadBuilder(PayloadBuilder builder);
        void onJoined(JoinedCallback callback);
        void onDownlink(DownlinkCallback callback);
        bool send(uint8_t* data, size_t size) override;

    private:
        RadioPins pins;

        uint8_t devEui[8];
        uint8_t appEui[8];
        uint8_t appKey[16];

        bool joined = false;

        uint32_t autoUplinkInterval = 60;
        PayloadBuilder payloadBuilder = nullptr;


        JoinedCallback joinedCb = nullptr;
        DownlinkCallback downlinkCb = nullptr;

        osjob_t sendjob;

        static constexpr uint8_t MAX_PAYLOAD_LEN = 51;

        uint8_t readRegister(uint8_t address);
        void handleEvent(ev_t event);
        void handleAutoSend();
        static void onEventRelay(ev_t event);
        static void doSendRelay(osjob_t* job);
        friend void os_getDevEui(u1_t* buf);
        friend void os_getArtEui(u1_t* buf);
        friend void os_getDevKey(u1_t* buf);
        friend void onEvent(ev_t event);
};


inline LoRaWANCommunication lorawan;

extern "C"
{
    void os_getDevEui(u1_t* buf)
    {
        memcpy(buf,lorawan.devEui,8);
    }


    void os_getArtEui(u1_t* buf)
    {
        memcpy(buf, lorawan.appEui, 8);
    }


    void os_getDevKey(u1_t* buf)
    {
        memcpy(buf, lorawan.appKey, 16);
    }


    void onEvent(ev_t event)
    {
        lorawan.onEventRelay(event);
    }
}

// ============================================================
// Constructeur
// ============================================================

inline LoRaWANCommunication::LoRaWANCommunication(const RadioPins& pins, const uint8_t devEui[8], const uint8_t appEui[8], const uint8_t appKey[16])
    : pins(pins), Communication()
{
    memcpy(this->devEui, devEui, 8);
    memcpy(this->appEui, appEui, 8);
    memcpy(this->appKey, appKey, 16);
}


inline uint8_t LoRaWANCommunication::readRegister(uint8_t address)
{
    digitalWrite(pins.nss, LOW);
    SPI.transfer(address & 0x7F);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(pins.nss, HIGH);

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
    pinMode(pins.nss, OUTPUT);
    digitalWrite(pins.nss, HIGH);

    if (!checkModulePresence())
    {
        return false;
    }


    lmic_pinmap pinmap =
    {
        .nss = pins.nss,

        .rxtx = LMIC_UNUSED_PIN,

        .rst = pins.rst,

        .dio =
        {
            pins.dio0,
            pins.dio1,
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
// Configuration PayloadBuilder
// ============================================================

inline void LoRaWANCommunication::setPayloadBuilder(PayloadBuilder builder)
{
    payloadBuilder = builder;
}


// ============================================================
// Callback Join
// ============================================================

inline void LoRaWANCommunication::onJoined(JoinedCallback callback)
{
    joinedCb = callback;
}


// ============================================================
// Callback Downlink
// ============================================================

inline void LoRaWANCommunication::onDownlink(
    DownlinkCallback callback
)
{
    downlinkCb = callback;
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
// Relais événement LMIC
// ============================================================

inline void LoRaWANCommunication::onEventRelay(ev_t event)
{
    lorawan.handleEvent(event);
}

// ============================================================
// Relais envoi automatique
// ============================================================

inline void LoRaWANCommunication::doSendRelay(osjob_t* job)
{
    lorawan.handleAutoSend();
}


// ============================================================
// Gestion événements LMIC
// ============================================================

inline void LoRaWANCommunication::handleEvent(ev_t event)
{
    switch (event)
    {
        // ----------------------------------------------------
        // Join en cours
        // ----------------------------------------------------
        case EV_JOINING:
            break;

        // ----------------------------------------------------
        // Join réussi
        // ----------------------------------------------------
        case EV_JOINED:
            joined = true;

            LMIC_setLinkCheckMode(0);
            LMIC_setAdrMode(1);

            if (joinedCb)
            {
                joinedCb();
            }

            handleAutoSend();
            break;


        // ----------------------------------------------------
        // Transmission terminée
        // ----------------------------------------------------
        case EV_TXCOMPLETE:
            if (LMIC.dataLen && downlinkCb)
            {
                uint8_t port =LMIC.frame[LMIC.dataBeg - 1];
                downlinkCb(port, LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            }

            if (payloadBuilder)
            {
                os_setTimedCallback(
                    &sendjob,
                    os_getTime() +
                        sec2osticks(
                            autoUplinkInterval
                        ),
                    doSendRelay
                );
            }


            break;


        // ----------------------------------------------------
        // Join échoué
        // ----------------------------------------------------
        case EV_JOIN_FAILED:
            joined =false;
            break;


        // ----------------------------------------------------
        // Rejoin échoué
        // ----------------------------------------------------
        case EV_REJOIN_FAILED:
            joined = false;
            break;

        // ----------------------------------------------------
        // Autres événements
        // ----------------------------------------------------

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