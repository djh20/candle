#include "bluetooth_config.h"
#include "bluetooth.h"
#include "../config.h"
#include <BLEService.h>

void BluetoothConfig::begin()
{
  BLEService *configService = GlobalBluetooth.getServer()->createService(
    GlobalBluetooth.uuid(UUID_CUSTOM, BLE_SERVICE_CONFIG)
  );

  uint16_t vehicleId = GlobalConfig.getVehicleId();

  // TODO: Implement writing (saving changes to config).
  
  vehicleIdCharacteristic = configService->createCharacteristic(
    GlobalBluetooth.uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_CONFIG_VEHICLE_ID),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  vehicleIdCharacteristic->setValue(vehicleId);
  vehicleIdCharacteristic->setAccessPermissions(GlobalBluetooth.getAccessPermissions());

  configService->start();
}

BluetoothConfig GlobalBluetoothConfig;