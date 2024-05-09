#pragma once

#include <BLEServer.h>

class BluetoothManager: public BLEServerCallbacks 
{
  public:
    BluetoothManager();

    void begin();
    void setCanAdvertise(bool value);
    bool getDeviceConnected();

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    void loop();

    BLEServer* server;

  private:
    bool advertising = false;
    bool canAdvertise = true;
    bool deviceConnected = false;
    uint32_t lastDisconnectMillis;

    BLESecurity* security;
};

extern BluetoothManager GlobalBluetoothManager;
