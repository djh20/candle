#include "bluetooth_device_info.h"
#include "bluetooth.h"
#include <BLEService.h>

void BluetoothDeviceInfo::begin()
{
  BLEService *deviceInfoService = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_STANDARD, BLE_STD_SERVICE_DEVICE_INFO)
  );
  
  BLECharacteristic *hardwareModelCharacteristic = deviceInfoService->createCharacteristic(
    Bluetooth::uuid(UUID_STANDARD, BLE_STD_CHARACTERISTIC_MODEL_NUMBER),
    BLECharacteristic::PROPERTY_READ
  );
  hardwareModelCharacteristic->setValue(HARDWARE_MODEL);
  hardwareModelCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());

  BLECharacteristic *firmwareVersionCharacteristic = deviceInfoService->createCharacteristic(
    Bluetooth::uuid(UUID_STANDARD, BLE_STD_CHARACTERISTIC_FIRMWARE_VERSION),
    BLECharacteristic::PROPERTY_READ
  );
  firmwareVersionCharacteristic->setValue(FIRMWARE_VERSION);
  firmwareVersionCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());

  deviceInfoService->start();
}
