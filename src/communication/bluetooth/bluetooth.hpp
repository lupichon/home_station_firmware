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
        String utcOffsetUUID; 

        BLEServer* server;
        BLEService* service;
        BLECharacteristic* measurementCharacteristic;
        BLECharacteristic* timeSyncCharacteristic;
        BLECharacteristic* alarmTargetCharacteristic;
        BLECharacteristic* utcOffsetCharacteristic;

        bool newTimeSync = false;
        uint32_t timeSyncValue = 0;
        bool newAlarmTarget = false;
        uint32_t alarmTargetValue = 0;
        bool newUtcOffset = false;
        int32_t utcOffsetValue = 0;

    public:

        BluetoothCommunication() = default;
        BluetoothCommunication(const String& name, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID, const String& utcOffsetUUID);

        bool begin();
        bool send(uint8_t* data, size_t dataSize) override;
        bool hasConnectedClient() const;
        void onConnect(BLEServer* server) override;
        void onDisconnect(BLEServer* server) override;
        void onWrite(BLECharacteristic* characteristic) override;
        bool hasNewTimeSync() const;
        uint32_t consumeTimeSync();
        bool hasNewAlarmTarget() const;
        uint32_t consumeAlarmTarget();
        bool hasNewUtcOffset() const;
        int32_t consumeUtcOffset();
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

inline BluetoothCommunication::BluetoothCommunication(const String& name, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID, const String& utcOffsetUUID)
    : deviceName(name),
      serviceUUID(serviceUUID),
      characteristicUUID(characteristicUUID),
      timeSyncUUID(timeSyncUUID),
      alarmTargetUUID(alarmTargetUUID),
      utcOffsetUUID(utcOffsetUUID),
      server(nullptr),
      service(nullptr),
      measurementCharacteristic(nullptr),
      timeSyncCharacteristic(nullptr),
      alarmTargetCharacteristic(nullptr),
      utcOffsetCharacteristic(nullptr),
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

    utcOffsetCharacteristic = service->createCharacteristic(
        BLEUUID(utcOffsetUUID.c_str()),
        BLECharacteristic::PROPERTY_WRITE
    );
    if (utcOffsetCharacteristic == nullptr) return false;
    utcOffsetCharacteristic->setCallbacks(this);

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

inline void BluetoothCommunication::onWrite(BLECharacteristic* characteristic)
{
    String value = characteristic->getValue();

    if (characteristic == timeSyncCharacteristic)
    {
        if (value.length() != sizeof(uint32_t)) return;
        memcpy(&timeSyncValue, value.c_str(), sizeof(uint32_t));
        newTimeSync = true;
    }
    else if (characteristic == alarmTargetCharacteristic)
    {
        if (value.length() != sizeof(uint32_t)) return;
        memcpy(&alarmTargetValue, value.c_str(), sizeof(uint32_t));
        newAlarmTarget = true;
    }
    else if (characteristic == utcOffsetCharacteristic)
    {
        if (value.length() != sizeof(int8_t)) return;
        utcOffsetValue = (int8_t)value[0];
        newUtcOffset = true;
    }
}

inline bool BluetoothCommunication::hasNewTimeSync() const
{
    return newTimeSync;
}
 
inline uint32_t BluetoothCommunication::consumeTimeSync()
{
    newTimeSync = false;
    return timeSyncValue;
}

inline bool BluetoothCommunication::hasNewAlarmTarget() const
{
    return newAlarmTarget;
}
 
inline uint32_t BluetoothCommunication::consumeAlarmTarget()
{
    newAlarmTarget = false;
    return alarmTargetValue;
}

inline bool BluetoothCommunication::hasNewUtcOffset() const
{
    return newUtcOffset;
}

inline int32_t BluetoothCommunication::consumeUtcOffset()
{
    newUtcOffset = false;
    return utcOffsetValue;
}

