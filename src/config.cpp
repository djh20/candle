#include "config.h"
#include "bluetooth/bluetooth.h"

#define RW_MODE false
#define RO_MODE true

#define CONFIG_ID "config"

#define CONFIG_KEY_VEHICLE_ID "vehicle_id"
#define CONFIG_KEY_DEVICE_NAME "device_name"
#define CONFIG_KEY_BLUETOOTH_MODE "bluetooth_mode"
#define CONFIG_KEY_BLUETOOTH_PIN "bluetooth_pin"

void Config::begin()
{
  log_i("Loading config");

  prefs.begin(CONFIG_ID, RO_MODE);

  bluetoothMode = prefs.getUChar(CONFIG_KEY_BLUETOOTH_MODE);

  // If no device name is stored, generate a unique name based on esp mac address.
  if (!prefs.isKey(CONFIG_KEY_DEVICE_NAME))
  {
    uint8_t mac[8];
    esp_efuse_mac_get_default(mac); // Read MAC address from eFuse.

    // ASCII:
    // Number characters from 48 to 57.
    // Letter characters from 65 to 90.

    // Calculate letters using 5th byte of MAC address.
    deviceName[7] = ((mac[4] >> 4) & 0x0F) + 65;
    deviceName[8] = (mac[4] & 0x0F) + 65;

    // Calculate numbers using 6th byte of MAC address.
    deviceName[9] = ((mac[5] >> 3) & 0x07) + 48;
    deviceName[10] = (mac[5] & 0x07) + 48;

    log_i("Generated device name: %s", deviceName);
  }
  else
  {
    prefs.getString(CONFIG_KEY_DEVICE_NAME, deviceName, CONFIG_DEVICE_NAME_MAX_LEN);
  }

  bluetoothPin = prefs.getUInt(CONFIG_KEY_BLUETOOTH_PIN);
  vehicleId = prefs.getUShort(CONFIG_KEY_VEHICLE_ID, 0x5CB5); // Default to Nissan Leaf (change this later).

  prefs.end();
}

void Config::writeBluetoothMode(uint8_t mode) 
{
  prefs.begin(CONFIG_ID, RW_MODE);
  prefs.putUChar(CONFIG_KEY_BLUETOOTH_MODE, mode);
  prefs.end();
}

uint8_t Config::getBluetoothMode()
{
  return bluetoothMode;
}

void Config::writeDeviceName(const char *name)
{
  prefs.begin(CONFIG_ID, RW_MODE);
  prefs.putString(CONFIG_KEY_DEVICE_NAME, name);
  prefs.end();
}

char *Config::getDeviceName() 
{
  return deviceName;
}

void Config::writeBluetoothPin(uint32_t pin)
{
  prefs.begin(CONFIG_ID, RW_MODE);
  prefs.putUInt(CONFIG_KEY_BLUETOOTH_PIN, pin);
  prefs.end();
}

uint32_t Config::getBluetoothPin()
{
  return bluetoothPin;
}

void Config::writeVehicleId(uint16_t id)
{
  prefs.begin(CONFIG_ID, RW_MODE);
  prefs.putUShort(CONFIG_KEY_VEHICLE_ID, id);
  prefs.end();
}

uint16_t Config::getVehicleId() 
{
  return vehicleId;
}

Config GlobalConfig;