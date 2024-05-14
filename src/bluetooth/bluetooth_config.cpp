#include "bluetooth_config.h"
#include "bluetooth.h"
#include "../config.h"
#include <BLEService.h>

static BLECharacteristic *vehicleIdCharacteristic;
static BLECharacteristic *bluetoothNameCharacteristic;

void BluetoothConfig::begin()
{
  BLEService *configService = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_CUSTOM, BLE_SERVICE_CONFIG)
  );

  uint16_t vehicleId = Config::getVehicleId();

  // TODO: Implement writing (saving changes to config).
  
  vehicleIdCharacteristic = configService->createCharacteristic(
    Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_CONFIG_VEHICLE_ID),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  vehicleIdCharacteristic->setValue(vehicleId);
  vehicleIdCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());

  bluetoothNameCharacteristic = configService->createCharacteristic(
    Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_CONFIG_BLUETOOTH_NAME),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  bluetoothNameCharacteristic->setValue(Config::getBluetoothName());
  bluetoothNameCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());

  configService->start();
}
