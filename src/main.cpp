#include <Arduino.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "utils/logger.h"
#include "config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "vehicle/vehicle_catalog.h"

#define SEND_INTERVAL 60U

//#define TEST_MODE
#define TEST_UPDATE_INTERVAL 500U

#define CONFIG_SERVICE_UUID "210a923d-927f-4c3b-ac85-49d60ce337e0"
#define CATALOG_SERVICE_UUID "900e43d1-3436-4264-a166-201133f6337a"

Vehicle* currentVehicle;

uint32_t lastSendMillis = 0;
uint32_t nextTestUpdateMillis = 0;
uint32_t startAdvertisingMillis = 0;

bool bleDeviceConnected = false;
bool bleAdvertising = false;
BLEServer* bleServer;

BLECharacteristic *configVehicleId;

class BluetoothCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bleDeviceConnected = true;
    bleAdvertising = false;
    Logger.log(Info, "ble", "Device connected");
  };

  void onDisconnect(BLEServer* pServer) {
    bleDeviceConnected = false;
    startAdvertisingMillis = millis() + 500;
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
    if (!bleAdvertising && startAdvertisingMillis >= now) {
      bleAdvertising = true;
      BLEDevice::startAdvertising();
    }

    if (now - lastSendMillis >= SEND_INTERVAL && bleDeviceConnected) {
      currentVehicle->sendUpdatedMetrics(lastSendMillis);
      lastSendMillis = now;
    }

    #ifdef TEST_MODE
    if (now >= nextTestUpdateMillis) {
      VehicleNissanLeaf *leaf = (VehicleNissanLeaf*) currentVehicle;

      leaf->awake->setValue(1);
      leaf->gear->setValue(3);
      leaf->soc->setValue(85.5);
      leaf->range->setValue(104);
      leaf->batteryTemp->setValue(43);
      leaf->batteryCapacity->setValue(27);
      leaf->soh->setValue(87.32);

      leaf->parkBrake->setValue(!leaf->parkBrake->value);
      leaf->headlights->setValue(!leaf->headlights->value);

      // float gearValue = leaf->gear->value;
      // gearValue += 1;
      // if (gearValue > 3) gearValue = 0;
      // leaf->gear->setValue(gearValue);

      float speedValue = leaf->speed->value;
      speedValue += 5;
      if (speedValue > 100) speedValue = 0;
      leaf->speed->setValue(speedValue);

      float powerValue = leaf->batteryPower->value;
      powerValue += 5;
      if (powerValue > 80) powerValue = 0;
      leaf->batteryPower->setValue(powerValue);

      float steeringValue = leaf->steeringAngle->value;
      steeringValue += 0.2;
      if (steeringValue > 1) steeringValue = -1;
      leaf->steeringAngle->setValue(steeringValue);
      
      nextTestUpdateMillis = now + TEST_UPDATE_INTERVAL;
    }
    #endif
  }
}