/**
 * @file    bluetooth.hpp
 * @brief   Bluetooth communication interface for the HomeStation device.
 * @author  Lucas Pichon
 * @date    2026-07-25
 */

#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "../communication.hpp"

// ============================================================
// BluetoothCommunication class definition
// ============================================================

class BluetoothCommunication : public Communication, public BLEServerCallbacks, public BLECharacteristicCallbacks
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        String deviceName;          // Bluetooth device name
        String serviceUUID;         // Bluetooth service UUID
        String characteristicUUID;  // Bluetooth characteristic UUID for measurements
        String timeSyncUUID;        // Bluetooth characteristic UUID for time synchronization
        String alarmTargetUUID;     // Bluetooth characteristic UUID for alarm target

        BLEServer* server;      // Pointer to the BLE server
        BLEService* service;    // Pointer to the BLE service

        BLECharacteristic* measurementCharacteristic;   // Pointer to the BLE characteristic for measurements
        BLECharacteristic* timeSyncCharacteristic;      // Pointer to the BLE characteristic for time synchronization
        BLECharacteristic* alarmTargetCharacteristic;   // Pointer to the BLE characteristic for alarm target

        std::function<void(uint32_t)> onTimeSyncReceived;       // Callback for time synchronization
        std::function<void(uint32_t)> onAlarmTargetReceived;    // Callback for alarm target updates

    // ── Public interface ────────────────────────────────────────────────────
    public:
        /**
         * @brief Constructor for BluetoothCommunication.
         */
        BluetoothCommunication();

        /**
         * @brief Configure the BLE communication parameters before initialization.
         * @param deviceName         BLE device name.
         * @param serviceUUID        BLE service UUID.
         * @param characteristicUUID BLE characteristic UUID for measurements.
         * @param timeSyncUUID       BLE characteristic UUID for time synchronization.
         * @param alarmTargetUUID    BLE characteristic UUID for alarm target.
         */
        void configure(const String& deviceName, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID);

        /**
         * @brief Initialize the Bluetooth communication.
         * @return true if initialization was successful, false otherwise.
         */
        bool begin() override;

        /**
         * @brief Send data over Bluetooth.
         * @param data Pointer to the data buffer.
         * @param dataSize Size of the data to send.
         * @return true if data was sent successfully, false otherwise.
         */
        bool send(uint8_t* data, size_t dataSize) override;

        /**
         * @brief Check if there is a connected Bluetooth client.
         * @return true if a client is connected, false otherwise.
         */
        bool hasConnectedClient() const;

        /**
         * @brief Set the callback for time synchronization.
         * @param cb Callback function to handle time synchronization.
         */
        void setTimeSyncCallback(std::function<void(uint32_t)> cb);

        /**
         * @brief Set the callback for alarm target updates.
         * @param cb Callback function to handle alarm target updates.
         */
        void setAlarmTargetCallback(std::function<void(uint32_t)> cb);

        void onConnect(BLEServer* server) override;
        void onDisconnect(BLEServer* server) override;
        void onWrite(BLECharacteristic* characteristic) override;
};

// ============================================================
// implementations of BluetoothCommunication methods
// ============================================================

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────────────────────

inline void BluetoothCommunication::configure(const String& deviceName, const String& serviceUUID, const String& characteristicUUID, const String& timeSyncUUID, const String& alarmTargetUUID)
{
    // Store the provided configuration parameters
    this->deviceName = deviceName;
    this->serviceUUID = serviceUUID;
    this->characteristicUUID = characteristicUUID;
    this->timeSyncUUID = timeSyncUUID;
    this->alarmTargetUUID = alarmTargetUUID;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialization
// ─────────────────────────────────────────────────────────────────────────────

inline bool BluetoothCommunication::begin()
{
    // Initialize the BLE device
    BLEDevice::init(deviceName.c_str());
    
    // Create the BLE server
    server = BLEDevice::createServer();
    if (server == nullptr) return false;

    // Create the BLE service 
    service = server->createService(BLEUUID(serviceUUID.c_str()));
    if (service == nullptr) return false;

    // Set the server callbacks to handle client connections and disconnections
    server->setCallbacks(this);

    // Create the BLE characteristic for measurements with read and notify properties
    measurementCharacteristic = service->createCharacteristic(
        BLEUUID(characteristicUUID.c_str()),
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    if (measurementCharacteristic == nullptr) return false;

    measurementCharacteristic->addDescriptor(new BLE2902());

    // Create the BLE characteristic for time synchronization with write property
    timeSyncCharacteristic = service->createCharacteristic(
        BLEUUID(timeSyncUUID.c_str()),
        BLECharacteristic::PROPERTY_WRITE
    );
    if (timeSyncCharacteristic == nullptr) return false;
    timeSyncCharacteristic->setCallbacks(this);

    // Create the BLE characteristic for alarm target with write property
    alarmTargetCharacteristic = service->createCharacteristic(
        BLEUUID(alarmTargetUUID.c_str()),
        BLECharacteristic::PROPERTY_WRITE
    );
    if (alarmTargetCharacteristic == nullptr) return false;
    alarmTargetCharacteristic->setCallbacks(this);

    // Start the BLE service
    service->start();

    // Start advertising the BLE service
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(BLEUUID(serviceUUID.c_str()));
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    initialized = true;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Data Transmission
// ─────────────────────────────────────────────────────────────────────────────

inline bool BluetoothCommunication::send(uint8_t* data, size_t dataSize)
{
    if (!initialized || measurementCharacteristic == nullptr)
    {
        return false;
    }

    // Set the value of the measurement characteristic and notify connected clients
    measurementCharacteristic->setValue(
        data, dataSize
    );

    measurementCharacteristic->notify();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Client Connection Check
// ─────────────────────────────────────────────────────────────────────────────

inline bool BluetoothCommunication::hasConnectedClient() const
{
    if (!initialized || server == nullptr)
    {
        return false;
    }

    // Check if there is at least one connected client
    return server->getConnectedCount() > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Callbacks wiring
// ─────────────────────────────────────────────────────────────────────────────

inline void BluetoothCommunication::setTimeSyncCallback(std::function<void(uint32_t)> cb)
{
    // Store the provided callback function for time synchronization
    onTimeSyncReceived = cb;
}

inline void BluetoothCommunication::setAlarmTargetCallback(std::function<void(uint32_t)> cb)
{
    // Store the provided callback function for alarm target updates
    onAlarmTargetReceived = cb;
}

// ─────────────────────────────────────────────────────────────────────────────
// BLE Server Callbacks
// ─────────────────────────────────────────────────────────────────────────────

inline void BluetoothCommunication::onConnect(BLEServer* server)
{
    // Handle client connection
    BLEServerCallbacks::onConnect(server);
}

inline void BluetoothCommunication::onDisconnect(BLEServer* server)
{
    // Handle client disconnection and restart advertising
    BLEServerCallbacks::onDisconnect(server);
    BLEDevice::startAdvertising(); 
}

inline void BluetoothCommunication::onWrite(BLECharacteristic* characteristic)
{
    String value = characteristic->getValue();

    if (characteristic == timeSyncCharacteristic)   // Check if the written characteristic is for time synchronization
    {
        if (value.length() != sizeof(uint32_t)) return;

        // Convert the received value to a uint32_t epoch time
        uint32_t epoch;
        memcpy(&epoch, value.c_str(), sizeof(uint32_t));

        if (onTimeSyncReceived)  // Call the callback function for time synchronization if it is set
        {
            onTimeSyncReceived(epoch);
        }
    }

    else if (characteristic == alarmTargetCharacteristic) // Check if the written characteristic is for alarm target
    {
        if (value.length() != sizeof(uint32_t)) return;

        // Convert the received value to a uint32_t target epoch time
        uint32_t targetEpoch;
        memcpy(&targetEpoch, value.c_str(), sizeof(uint32_t));

        if (onAlarmTargetReceived)  // Call the callback function for alarm target updates if it is set
        {
            onAlarmTargetReceived(targetEpoch);
        }
    }
}