#include "bluetooth_device_info.h"
#include "bluetooth.h"
#include <BLEService.h>

void BluetoothDeviceInfo::begin()
{
  BLEService *deviceInfoService = GlobalBluetooth.getServer()->createService(
    GlobalBluetooth.uuid(UUID_STANDARD, BLE_STD_SERVICE_DEVICE_INFO)
  );
  
  BLECharacteristic *hardwareModelCharacteristic = deviceInfoService->createCharacteristic(
    GlobalBluetooth.uuid(UUID_STANDARD, BLE_STD_CHARACTERISTIC_MODEL_NUMBER),
    BLECharacteristic::PROPERTY_READ
  );
  hardwareModelCharacteristic->setValue(HARDWARE_MODEL);
  hardwareModelCharacteristic->setAccessPermissions(GlobalBluetooth.getAccessPermissions());

  BLECharacteristic *firmwareVersionCharacteristic = deviceInfoService->createCharacteristic(
    GlobalBluetooth.uuid(UUID_STANDARD, BLE_STD_CHARACTERISTIC_FIRMWARE_VERSION),
    BLECharacteristic::PROPERTY_READ
  );
  firmwareVersionCharacteristic->setValue(FIRMWARE_VERSION);
  firmwareVersionCharacteristic->setAccessPermissions(GlobalBluetooth.getAccessPermissions());

  deviceInfoService->start();
}

BluetoothDeviceInfo GlobalBluetoothDeviceInfo;