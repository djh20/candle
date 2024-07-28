#include "bluetooth_device_info.h"
#include "bluetooth_manager.h"
#include <BLEService.h>

void BluetoothDeviceInfo::begin()
{
  BLEService *deviceInfoService = GlobalBluetoothManager.getServer()->createService(
    GlobalBluetoothManager.uuid(UUID_STANDARD, BLE_STD_S_DEVICE_INFO)
  );
  
  BLECharacteristic *hardwareModelCharacteristic = deviceInfoService->createCharacteristic(
    GlobalBluetoothManager.uuid(UUID_STANDARD, BLE_STD_C_MODEL_NUMBER),
    BLECharacteristic::PROPERTY_READ
  );
  hardwareModelCharacteristic->setValue(HARDWARE_MODEL);
  hardwareModelCharacteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());

  BLECharacteristic *firmwareVersionCharacteristic = deviceInfoService->createCharacteristic(
    GlobalBluetoothManager.uuid(UUID_STANDARD, BLE_STD_C_FIRMWARE_VERSION),
    BLECharacteristic::PROPERTY_READ
  );
  firmwareVersionCharacteristic->setValue(FIRMWARE_VERSION);
  firmwareVersionCharacteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());

  deviceInfoService->start();
}

BluetoothDeviceInfo GlobalBluetoothDeviceInfo;