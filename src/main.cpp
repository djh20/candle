#include <Arduino.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "utils/logger.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "vehicle/vehicle_catalog.h"

#define SEND_INTERVAL 50U

#define CONFIG_SERVICE_UUID "210a923d-927f-4c3b-ac85-49d60ce337e0"
#define CATALOG_SERVICE_UUID "900e43d1-3436-4264-a166-201133f6337a"

Vehicle* currentVehicle;

uint32_t lastSendMillis = 0;

bool bleDeviceConnected = false;
BLEServer* bleServer;

BLECharacteristic *configVehicleId;

class BluetoothCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleDeviceConnected = true;
    Logger.log(Info, "ble", "Device connected");
  };

  void onDisconnect(BLEServer* pServer) {
    bleDeviceConnected = false;
    BLEDevice::startAdvertising();
    Logger.log(Info, "ble", "Device disconnected");
  }
};

void selectVehicle(VehicleEntry &entry)
{
  currentVehicle = entry.createVehicle();
  currentVehicle->init(bleServer);

  configVehicleId->setValue(entry.id);
}

void setup() 
{
  Serial.begin(115200);
  delay(1000);

  // Print splash art & info
  Serial.println();
  Serial.println(
    "____ ____ _  _ ___  _    ____\n"
    "|    |__| |\\ | |  \\ |    |___\n"
    "|___ |  | | \\| |__/ |___ |___"
  );

  Serial.println("Version: DEV");
  Serial.println("Disclaimer: This is a work in progress.");

  Serial.println();

  BLEDevice::init("Candle");
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new BluetoothCallbacks());

  BLEService *configService = bleServer->createService(CONFIG_SERVICE_UUID);
  BLEService *catalogService = bleServer->createService(BLEUUID(CATALOG_SERVICE_UUID), 128, 0);

  configVehicleId = configService->createCharacteristic(
    BLEUUID((uint16_t) 0x0000),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  configVehicleId->addDescriptor(new BLE2902());

  for (uint16_t i = 0; i < VEHICLE_CATALOG_LENGTH; i++)
  {
    VehicleEntry entry = vehicleCatalog[i];
    Serial.println(entry.name);
    catalogService->createCharacteristic(
      BLEUUID(entry.id),
      BLECharacteristic::PROPERTY_READ
    )->setValue(entry.name);
  }

  configService->start();
  catalogService->start();
  
  BLEDevice::startAdvertising();

  selectVehicle(vehicleCatalog[0]);
}

void loop() 
{
  if (currentVehicle != NULL)
  {
    currentVehicle->update();

    uint32_t now = millis();
    if (now - lastSendMillis >= SEND_INTERVAL && bleDeviceConnected) {
      VehicleNissanLeaf *leaf = (VehicleNissanLeaf*) currentVehicle;
      leaf->gear->setValue(leaf->gear->value + 1, true);
      currentVehicle->sendUpdatedMetrics(lastSendMillis);

      float batteryPowerValue = leaf->batteryPower->value;
      batteryPowerValue += 5;
      if (batteryPowerValue > 100) batteryPowerValue = -100;

      leaf->batteryPower->setValue(batteryPowerValue, true);
      currentVehicle->sendUpdatedMetrics(lastSendMillis);

      lastSendMillis = now;
    }
  }
}