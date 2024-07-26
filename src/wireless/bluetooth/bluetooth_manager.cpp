#include "bluetooth_manager.h"
#include "../../config.h"
#include "../../vehicle/vehicle_manager.h"
#include "bluetooth_device_info.h"
#include "bluetooth_ota.h"
#include "bluetooth_metrics.h"
#include "bluetooth_console.h"
#include <BLEDevice.h>

#define ADVERTISE_DELAY 500

void BluetoothManager::begin()
{
  BLEDevice::init(GlobalConfig.hostname->getValue());
  
  server = BLEDevice::createServer();
  server->setCallbacks(this);

  bool encrypted = (GlobalConfig.getBluetoothMode() == BLE_MODE_ENCRYPTED);

  log_i("Bluetooth encryption is %s", encrypted ? "enabled" : "disabled");

  if (encrypted)
  {
    BLESecurity *security = new BLESecurity();
    security->setStaticPIN(GlobalConfig.blePin->getValue());
    security->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
    security->setCapability(ESP_IO_CAP_OUT);

    log_i("Bluetooth pin is %u", GlobalConfig.blePin->getValue());
  }

  GlobalBluetoothDeviceInfo.begin();
  GlobalBluetoothOTA.begin();
  GlobalBluetoothConsole.begin();
  // GlobalBluetoothMetrics.begin();
}

void BluetoothManager::loop()
{
  GlobalBluetoothOTA.loop();
  // GlobalBluetoothMetrics.loop();
  
  uint32_t now = millis();
  if (!advertising && !clientConnected && enabled && now - lastDisconnectMillis >= ADVERTISE_DELAY)
  {
    BLEDevice::startAdvertising();
    advertising = true;
    log_i("Started bluetooth advertising");
  }
  else if (advertising && !enabled)
  {
    BLEDevice::stopAdvertising();
    advertising = false;
    log_i("Stopped bluetooth advertising");
  }
}

void BluetoothManager::onConnect(BLEServer* pServer) {
  clientConnected = true;
  advertising = false;
  log_i("Bluetooth client connected");
}

void BluetoothManager::onDisconnect(BLEServer* pServer) {
  lastDisconnectMillis = millis();
  clientConnected = false;
  log_i("Bluetooth client disconnected");
}

void BluetoothManager::setEnabled(bool enabled)
{
  if (enabled == this->enabled) return;

  log_i("Bluetooth is now %s", enabled ? "enabled" : "disabled");

  if (!enabled) disconnectClient();
  
  this->enabled = enabled;
}

// void BluetoothManager::setCanAdvertise(bool value)
// {
//   canAdvertise = value;
// }

BLEServer *BluetoothManager::getServer()
{
  return server;
}

bool BluetoothManager::getClientConnected()
{
  return clientConnected;
}

BLEUUID BluetoothManager::uuid(bool custom, uint16_t id, uint16_t discriminator) 
{
  if (custom)
  {
    uuidBuffer[2] = id >> 8;
    uuidBuffer[3] = id;

    uuidBuffer[4] = discriminator >> 8;
    uuidBuffer[5] = discriminator;

    return BLEUUID(uuidBuffer, 16, true);
  }
  else
  {
    return BLEUUID(id);
  }
}

esp_gatt_perm_t BluetoothManager::getAccessPermissions()
{
  if (GlobalConfig.getBluetoothMode() == BLE_MODE_ENCRYPTED)
  {
    return ESP_GATT_PERM_READ_ENC_MITM | ESP_GATT_PERM_WRITE_ENC_MITM;
  }
  else
  {
    return ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
  }
}

void BluetoothManager::disconnectClient()
{
  if (server->getConnectedCount() > 0)
  {
    server->disconnect(server->getConnId());
  }
}

BluetoothManager GlobalBluetoothManager;