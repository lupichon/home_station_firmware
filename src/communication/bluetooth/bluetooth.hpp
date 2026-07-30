#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "../communication.hpp"
#include "bluetooth_UUID.hpp"


class BluetoothCommunication : public Communication, public BLEServerCallbacks
{
    private:

        const char* deviceName;

        BLEServer* server;
        BLEService* service;
        BLECharacteristic* measurementCharacteristic;

    public:

        BluetoothCommunication(const char* name);

        bool begin();
        bool send(uint8_t* data, size_t dataSize) override;
        bool hasConnectedClient() const;
        void onConnect(BLEServer* server) override;
        void onDisconnect(BLEServer* server) override;
};

inline void BluetoothCommunication::onConnect(BLEServer* server)
{
    // Handle client connection
    BLEServerCallbacks::onConnect(server);
}

inline void BluetoothCommunication::onDisconnect(BLEServer* server)
{
    BLEServerCallbacks::onDisconnect(server);
    BLEDevice::startAdvertising(); // Restart advertising when a client disconnects
}


// ============================================================
// Constructor
// ============================================================

inline BluetoothCommunication::BluetoothCommunication(const char* name)
    : deviceName(name),
      server(nullptr),
      service(nullptr),
      measurementCharacteristic(nullptr),
      Communication()
{

}


// ============================================================
// Initialization
// ============================================================

inline bool BluetoothCommunication::begin()
{
    BLEDevice::init(deviceName);


    // Create BLE server
    server = BLEDevice::createServer();

    if (server == nullptr)
    {
        return false;
    }


    // Create HomeStation service
    service = server->createService(
        BluetoothUUID::SERVICE
    );
    if (service == nullptr)
    {
        return false;
    }
    server->setCallbacks(this);


    // Create measurement characteristic
    measurementCharacteristic =
        service->createCharacteristic(
            BluetoothUUID::MEASUREMENT,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );


    if (measurementCharacteristic == nullptr)
    {
        return false;
    }


    // Enable notifications
    measurementCharacteristic->addDescriptor(
        new BLE2902()
    );


    // Start BLE service
    service->start();


    // Start advertising
    BLEAdvertising* advertising =
        BLEDevice::getAdvertising();

    advertising->addServiceUUID(
        BluetoothUUID::SERVICE
    );

    advertising->setScanResponse(true);

    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);

    //advertising->setMinInterval(16000);
    //advertising->setMaxInterval(16000);

    BLEDevice::startAdvertising();


    initialized = true;

    return true;
}

// ============================================================
// Send data
// ============================================================

inline bool BluetoothCommunication::send(uint8_t* data, size_t dataSize)
{
    if (!initialized || measurementCharacteristic == nullptr)
    {
        return false;
    }


    measurementCharacteristic->setValue(
        data, dataSize
    );

    measurementCharacteristic->notify();
    return true;
}

inline bool BluetoothCommunication::hasConnectedClient() const
{
    if (!initialized || server == nullptr)
    {
        return false;
    }

    return server->getConnectedCount() > 0;
}
