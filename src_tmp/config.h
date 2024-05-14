#pragma once

#include <Preferences.h>
#include "bluetooth/uuid.h"

class PersistentConfig 
{
  public:
    PersistentConfig();

    void load(const char* id);
    
    bool getStatus();
    void setStatus(bool newStatus);

    char* getBluetoothName();
    void setBluetoothName(const char* newName);

    // uint32_t getBluetoothPin();
    // void setBluetoothPin(uint32_t newPin);
  
  private:
    const char* id;
    Preferences* prefs;

    bool status = false;
    char bluetoothName[BLE_NAME_MAX_LEN] = "CANDLE-";
    // uint32_t bluetoothPin;
};

extern PersistentConfig Config;
