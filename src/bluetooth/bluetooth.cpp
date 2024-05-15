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

// Custom UUID Format (Randomly Generated):
// XXXXXXXX-XXXX-4D91-B049-E2828E6DA1A0
static uint8_t uuidBuffer[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
  0x4D, 0x91,
  0xB0, 0x49,
  0xE2, 0x82, 0x8E, 0x6D, 0xA1, 0xA0
};

static BLEServer *server;
static bool clientConnected = false;
static bool advertising = false;
static bool canAdvertise = true;
static uint32_t lastDisconnectMillis = 0;

class BluetoothCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    clientConnected = true;
    advertising = false;
    log_i("Bluetooth client connected");
  };

  void onDisconnect(BLEServer* pServer) {
    lastDisconnectMillis = millis();
    clientConnected = false;
    log_i("Bluetooth client disconnected");
  }
};

void Bluetooth::begin()
{
  BLEDevice::init(Config::getBluetoothName());
  
  server = BLEDevice::createServer();
  server->setCallbacks(new BluetoothCallbacks());

  if (Config::getBluetoothMode() == BLUETOOTH_MODE_ENCRYPTED)
  {
    BLESecurity *security = new BLESecurity();
    security->setStaticPIN(Config::getBluetoothPin());
    security->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    security->setCapability(ESP_IO_CAP_OUT);
  }

  BluetoothDeviceInfo::begin();
  BluetoothOTA::begin();
  BluetoothConfig::begin();
  BluetoothVehicle::begin();
}

void Bluetooth::loop()
{
  BluetoothVehicle::loop();
  
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
  if (Config::getBluetoothMode() == BLUETOOTH_MODE_ENCRYPTED)
  {
    return ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED;
  }
  else
  {
    return ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
  }
}