#include "bluetooth.h"
#include "../config.h"
#include "bluetooth_device_info.h"
#include "bluetooth_ota.h"
#include "bluetooth_config.h"
#include "bluetooth_vehicle.h"
#include <BLEDevice.h>

#define ADVERTISE_DELAY 500U

#define BLUETOOTH_MODE_OPEN 0U
#define BLUETOOTH_MODE_ENCRYPTED 1U

void Bluetooth::begin()
{
  BLEDevice::init(GlobalConfig.getDeviceName());
  
  server = BLEDevice::createServer();
  server->setCallbacks(this);

  if (GlobalConfig.getBluetoothMode() == BLUETOOTH_MODE_ENCRYPTED)
  {
    BLESecurity *security = new BLESecurity();
    security->setStaticPIN(GlobalConfig.getBluetoothPin());
    security->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    security->setCapability(ESP_IO_CAP_OUT);
  }

  GlobalBluetoothDeviceInfo.begin();
  GlobalBluetoothOTA.begin();
  GlobalBluetoothConfig.begin();
  GlobalBluetoothVehicle.begin();
}

void Bluetooth::loop()
{
  GlobalBluetoothOTA.loop();
  GlobalBluetoothVehicle.loop();
  
  uint32_t now = millis();
  if (!advertising && !clientConnected && canAdvertise && now - lastDisconnectMillis >= ADVERTISE_DELAY)
  {
    BLEDevice::startAdvertising();
    advertising = true;
    log_i("Started bluetooth advertising");
  }
  else if (advertising && !canAdvertise)
  {
    BLEDevice::stopAdvertising();
    advertising = false;
    log_i("Stopped bluetooth advertising");
  }
}

void Bluetooth::onConnect(BLEServer* pServer) {
  clientConnected = true;
  advertising = false;
  log_i("Bluetooth client connected");
}

void Bluetooth::onDisconnect(BLEServer* pServer) {
  lastDisconnectMillis = millis();
  clientConnected = false;
  log_i("Bluetooth client disconnected");
}

void Bluetooth::setCanAdvertise(bool value)
{
  canAdvertise = value;
}

BLEServer *Bluetooth::getServer()
{
  return server;
}

bool Bluetooth::getClientConnected()
{
  return clientConnected;
}

BLEUUID Bluetooth::uuid(bool custom, uint16_t id, uint16_t discriminator) 
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

esp_gatt_perm_t Bluetooth::getAccessPermissions()
{
  if (GlobalConfig.getBluetoothMode() == BLUETOOTH_MODE_ENCRYPTED)
  {
    return ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED;
  }
  else
  {
    return ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
  }
}

Bluetooth GlobalBluetooth;