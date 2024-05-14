#include "bluetooth_manager.h"
#include <BLEDevice.h>
#include "../utils/logger.h"
#include "../config/config.h"

#define ADVERTISE_DELAY 500U

BluetoothManager::BluetoothManager() : BLEServerCallbacks() {}

void BluetoothManager::begin()
{
  BLEDevice::init(GlobalConfig.getBluetoothName());

  server = BLEDevice::createServer();
  server->setCallbacks(this);

  // security = new BLESecurity();
  // security->setStaticPIN(123456);
  // security->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  // security->setCapability(ESP_IO_CAP_OUT);
}

bool BluetoothManager::getDeviceConnected()
{
  return deviceConnected;
}

void BluetoothManager::setCanAdvertise(bool value)
{
  canAdvertise = value;
}

void BluetoothManager::onConnect(BLEServer* server)
{
  deviceConnected = true;
  advertising = false;
  Logger.log(Info, "ble", "Device connected");
}

void BluetoothManager::onDisconnect(BLEServer* server)
{
  lastDisconnectMillis = millis();
  deviceConnected = false;
  Logger.log(Info, "ble", "Device disconnected");
}

void BluetoothManager::loop()
{
  uint32_t now = millis();
  if (!advertising && !deviceConnected && canAdvertise && now - lastDisconnectMillis >= ADVERTISE_DELAY)
  {
    BLEDevice::startAdvertising();
    advertising = true;
    Logger.log(Info, "ble", "Started advertising");
  }
  else if (advertising && !canAdvertise)
  {
    BLEDevice::stopAdvertising();
    advertising = false;
    Logger.log(Info, "ble", "Stopped advertising");
  }
}

BluetoothManager GlobalBluetoothManager;
