#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "../communication.hpp"


class BluetoothCommunication : public Communication, public BLEServerCallbacks, public BLECharacteristicCallbacks
{
    private:

        String deviceName;
        String serviceUUID; 
        String characteristicUUID;
        String timeSyncUUID;
        String alarmTargetUUID;

        BLEServer* server;
        BLEService* service;
        BLECharacteristic* measurementCharacteristic;
        BLECharacteristic* timeSyncCharacteristic;
        BLECharacteristic* alarmTargetCharacteristic;

        // Callbacks appelés directement depuis onWrite() (tâche BLE), pas depuis loop()
        std::function<void(uint32_t)> onTimeSyncReceived;
        std::function<void(uint32_t)> onAlarmTargetReceived;

    public:

        BluetoothCommunication();
        void configure(const String& deviceName, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID);
        bool begin() override;
        bool send(uint8_t* data, size_t dataSize) override;
        bool hasConnectedClient() const;
        void onConnect(BLEServer* server) override;
        void onDisconnect(BLEServer* server) override;
        void onWrite(BLECharacteristic* characteristic) override;
        void setTimeSyncCallback(std::function<void(uint32_t)> cb);
        void setAlarmTargetCallback(std::function<void(uint32_t)> cb);
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

inline BluetoothCommunication::BluetoothCommunication()
    :
      server(nullptr),
      service(nullptr),
      measurementCharacteristic(nullptr),
      timeSyncCharacteristic(nullptr),
      alarmTargetCharacteristic(nullptr),
      Communication()
{

}

void BluetoothCommunication::configure(const String& deviceName, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID)
{
    this->deviceName = deviceName;
    this->serviceUUID = serviceUUID;
    this->characteristicUUID = characteristicUUID;
    this->timeSyncUUID = timeSyncUUID;
    this->alarmTargetUUID = alarmTargetUUID;
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

    measurementCharacteristic = service->createCharacteristic(
        BLEUUID(characteristicUUID.c_str()),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    if (measurementCharacteristic == nullptr) return false;

    measurementCharacteristic->addDescriptor(new BLE2902());

    timeSyncCharacteristic = service->createCharacteristic(
        BLEUUID(timeSyncUUID.c_str()),
        BLECharacteristic::PROPERTY_WRITE
    );
    if (timeSyncCharacteristic == nullptr) return false;
    timeSyncCharacteristic->setCallbacks(this);

    alarmTargetCharacteristic = service->createCharacteristic(
        BLEUUID(alarmTargetUUID.c_str()),
        BLECharacteristic::PROPERTY_WRITE
    );
    if (alarmTargetCharacteristic == nullptr) return false;
    alarmTargetCharacteristic->setCallbacks(this);

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

// ============================================================
// Write handling — déclenche directement le callback correspondant
// (exécuté dans la tâche BLE, pas dans loop())
// ============================================================

inline void BluetoothCommunication::onWrite(BLECharacteristic* characteristic)
{
    String value = characteristic->getValue();

    if (characteristic == timeSyncCharacteristic)
    {
        if (value.length() != sizeof(uint32_t)) return;

        uint32_t epoch;
        memcpy(&epoch, value.c_str(), sizeof(uint32_t));

        if (onTimeSyncReceived)
        {
            onTimeSyncReceived(epoch);
        }
    }
    else if (characteristic == alarmTargetCharacteristic)
    {
        if (value.length() != sizeof(uint32_t)) return;

        uint32_t targetEpoch;
        memcpy(&targetEpoch, value.c_str(), sizeof(uint32_t));

        if (onAlarmTargetReceived)
        {
            onAlarmTargetReceived(targetEpoch);
        }
    }
}

// ============================================================
// Callback wiring
// ============================================================

inline void BluetoothCommunication::setTimeSyncCallback(std::function<void(uint32_t)> cb)
{
    onTimeSyncReceived = cb;
}

inline void BluetoothCommunication::setAlarmTargetCallback(std::function<void(uint32_t)> cb)
{
    onAlarmTargetReceived = cb;
}