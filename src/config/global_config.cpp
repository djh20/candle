#include "global_config.h"

#define RW_MODE false
#define RO_MODE true

#define CONFIG_KEY_STATUS "status"
#define CONFIG_KEY_BLE_NAME "ble_name"

Config::Config() {}

void Config::begin(const char* id)
{
  this->id = id;

  prefs = new Preferences();
  prefs->begin(id, RO_MODE);

  status = prefs->getBool(CONFIG_KEY_STATUS, false);

  // If no bluetooth name is stored, generate a unique name based on esp mac address.
  if (!prefs->isKey(CONFIG_KEY_BLE_NAME))
  {
    uint8_t mac[8];
    esp_efuse_mac_get_default(mac); // Read MAC address from eFuse.

    // ASCII:
    // Number characters from 48 to 57.
    // Letter characters from 65 to 90.

    // Calculate letters using 5th byte.
    bluetoothName[7] = ((mac[4] >> 4) & 0x0F) + 65;
    bluetoothName[8] = (mac[4] & 0x0F) + 65;

    // Calculate numbers using 6th byte.
    bluetoothName[9] = ((mac[5] >> 3) & 0x07) + 48;
    bluetoothName[10] = (mac[5] & 0x07) + 48;
  }
  else
  {
    prefs->getString(CONFIG_KEY_BLE_NAME, bluetoothName, BLE_NAME_MAX_LEN);
  }

  prefs->end();
}

bool Config::getStatus() 
{
  return status;
}

void Config::setStatus(bool newStatus)
{
  prefs->begin(id, RW_MODE);
  prefs->putBool(CONFIG_KEY_STATUS, newStatus);
  prefs->end();
}

char* Config::getBluetoothName() {
  return bluetoothName;
}

void Config::setBluetoothName(const char* newName) {
  prefs->begin(id, RW_MODE);
  prefs->putString(CONFIG_KEY_BLE_NAME, newName);
  prefs->end();
}

Config GlobalConfig;