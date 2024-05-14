#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

#include <Arduino.h>
#include "vehicle/vehicle_nissan_leaf.h"
#include "utils/logger.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "ble/uuid.h"
#include "ble/bluetooth_manager.h"
#include "vehicle/vehicle_manager.h"
#include "config/config.h"

#define SEND_INTERVAL 60U

// #define TEST_MODE
#define TEST_UPDATE_INTERVAL 500U

Vehicle* currentVehicle;

uint32_t lastSendMillis = 0;
uint32_t lastTestMillis = 0;

BLECharacteristic *configVehicleId;

void selectVehicle(VehicleEntry &entry)
{
  currentVehicle = entry.createVehicle();
  currentVehicle->init(GlobalBluetoothManager.server);

  configVehicleId->setValue(entry.id);
}

void setup() 
{
  GlobalConfig.begin("config");
  GlobalBluetoothManager.begin();
  GlobalVehicleManager.begin();

  delay(500);

  #ifdef SERIAL_ENABLE
  Serial.begin(115200);
  delay(1000);

  // Print splash art & info
  Serial.println();
  Serial.println(
    "____ ____ _  _ ___  _    ____\n"
    "|    |__| |\\ | |  \\ |    |___\n"
    "|___ |  | | \\| |__/ |___ |___"
  );

  Serial.println("Version: " FIRMWARE_VERSION);
  Serial.println("Disclaimer: This is a work in progress.");

  Serial.println();
  #endif

  BLEService *deviceInfoService = GlobalBluetoothManager.server->createService(BLEUUID((uint16_t)BLE_STD_SERVICE_DEVICE_INFO));
  deviceInfoService->createCharacteristic(
    BLEUUID((uint16_t)BLE_STD_CHARACTERISTIC_MODEL_NUMBER),
    BLECharacteristic::PROPERTY_READ
  )->setValue(HARDWARE_MODEL);

  deviceInfoService->createCharacteristic(
    BLEUUID((uint16_t)BLE_STD_CHARACTERISTIC_FIRMWARE_VERSION),
    BLECharacteristic::PROPERTY_READ
  )->setValue(FIRMWARE_VERSION);

  deviceInfoService->start();


  // BLEService *otaService = GlobalBluetoothManager.server->createService(generateUUID(BLE_SERVICE_OTA));
  
  // otaService->createCharacteristic(
  //   generateUUID(BLE_CHARACTERISTIC_OTA_COMMAND),
  //   BLECharacteristic::PROPERTY_READ |
  //   BLECharacteristic::PROPERTY_WRITE
  // );

  // otaService->createCharacteristic(
  //   generateUUID(BLE_CHARACTERISTIC_OTA_DATA),
  //   BLECharacteristic::PROPERTY_READ |
  //   BLECharacteristic::PROPERTY_WRITE
  // );

  // otaService->start();


  // BLEService *debugService = GlobalBluetoothManager.server->createService(generateUUID(BLE_SERVICE_DEBUG));
  
  // debugService->createCharacteristic(
  //   generateUUID(BLE_CHARACTERISTIC_DEBUG_COMMAND),
  //   BLECharacteristic::PROPERTY_READ |
  //   BLECharacteristic::PROPERTY_WRITE
  // );

  // debugService->createCharacteristic(
  //   generateUUID(BLE_CHARACTERISTIC_DEBUG_LOG),
  //   BLECharacteristic::PROPERTY_READ |
  //   BLECharacteristic::PROPERTY_NOTIFY
  // );

  // debugService->start();


  // BLEService *configService = GlobalBluetoothManager.server->createService(generateUUID(BLE_SERVICE_CONFIG));

  // configVehicleId = configService->createCharacteristic(
  //   generateUUID(BLE_CHARACTERISTIC_CONFIG_VEHICLE),
  //   BLECharacteristic::PROPERTY_READ
  // );
  
  // configService->start();

  // selectVehicle(vehicleCatalog[0]);
}

void loop() 
{
  GlobalBluetoothManager.loop();
  GlobalVehicleManager.loop();

  if (currentVehicle != NULL)
  {
    currentVehicle->update();

    uint32_t now = millis();

    if (now - lastSendMillis >= SEND_INTERVAL && GlobalBluetoothManager.getDeviceConnected()) {
      currentVehicle->sendUpdatedMetrics(lastSendMillis);
      lastSendMillis = now;
      
    }

    #ifdef TEST_MODE
    if (now - lastTestMillis >= TEST_UPDATE_INTERVAL) {
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
      
      lastTestMillis = now;
    }
    #endif
  }
}