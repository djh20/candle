#include "bluetooth_console.h"
#include "bluetooth_manager.h"
#include "../../console.h"

void BluetoothConsole::begin()
{
  BLEService *consoleService = GlobalBluetoothManager.getServer()->createService(
    GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_SERVICE_CONSOLE)
  );
  
  commandCharacteristic = consoleService->createCharacteristic(
    GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_CONSOLE_COMMAND),
    BLECharacteristic::PROPERTY_WRITE
  );
  commandCharacteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
  commandCharacteristic->setCallbacks(this);

  resultCharacteristic = consoleService->createCharacteristic(
    GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_CONSOLE_RESULT),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  resultCharacteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());

  consoleService->start();
}

void BluetoothConsole::onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) 
{
  if (pCharacteristic == commandCharacteristic)
  {
    log_i("Received bluetooth command (length: %u)",  pCharacteristic->getLength());
    GlobalConsole.processString((char *)pCharacteristic->getData());
  }
}

BluetoothConsole GlobalBluetoothConsole;