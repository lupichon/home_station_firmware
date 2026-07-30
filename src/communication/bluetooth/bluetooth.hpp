#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "../communication.hpp"


class BluetoothCommunication : public Communication, public BLEServerCallbacks
{
    private:

        String deviceName;
        String serviceUUID; 
        String characteristicUUID;

        BLEServer* server;
        BLEService* service;
        BLECharacteristic* measurementCharacteristic;

    public:

        BluetoothCommunication() = default;
        BluetoothCommunication(const String& name, const String& serviceUUID, const String& characteristicUUID);

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

inline BluetoothCommunication::BluetoothCommunication(const String& name, const String& serviceUUID, const String& characteristicUUID)
    : deviceName(name),
      serviceUUID(serviceUUID),
      characteristicUUID(characteristicUUID),
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
    BLEDevice::init(deviceName.c_str());
    
    server = BLEDevice::createServer();
    if (server == nullptr) return false;

    service = server->createService(BLEUUID(serviceUUID.c_str()));
    if (service == nullptr) return false;

    server->setCallbacks(this);

    Serial.println("BLE: create characteristic");
    measurementCharacteristic = service->createCharacteristic(
        BLEUUID(characteristicUUID.c_str()),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    if (measurementCharacteristic == nullptr) return false;

    measurementCharacteristic->addDescriptor(new BLE2902());

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(BLEUUID(serviceUUID.c_str()));
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
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
