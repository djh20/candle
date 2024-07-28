#pragma once

#include <BLEUUID.h>
#include <BLEServer.h>

#define UUID_STANDARD false
#define UUID_CUSTOM true

#define BLE_MODE_OPEN 0
#define BLE_MODE_ENCRYPTED 1

#define BLE_MAX_ATTRIBUTE_SIZE 256

#define BLE_STD_S_DEVICE_INFO 0x180A
#define BLE_STD_C_MODEL_NUMBER 0x2A24
#define BLE_STD_C_FIRMWARE_VERSION 0x2A26

#define BLE_S_OTA 0x0000
#define BLE_C_OTA_COMMAND 0x0001
#define BLE_C_OTA_DATA 0x0002

#define BLE_S_CONSOLE 0x0100
#define BLE_C_CONSOLE_COMMAND 0x0101
#define BLE_C_CONSOLE_RESULT 0x0102

#define BLE_S_METRICS 0x0200
#define BLE_C_GROUPED_METRIC_DATA 0x0201
#define BLE_D_GROUPED_METRIC_INFO 0x0202

class BluetoothManager: public BLEServerCallbacks
{
  public:
    void begin();
    void loop();

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    void setEnabled(bool enabled);
    // void setCanAdvertise(bool value);
    BLEServer *getServer();
    bool isClientConnected();
    BLEUUID uuid(bool custom, uint16_t id, uint16_t discriminator = 0x0000);
    esp_gatt_perm_t getAccessPermissions();
  
  private:
    void disconnectClient();

    BLEServer *server;
    bool enabled = false;
    bool clientConnected = false;
    bool advertising = false;
    // bool canAdvertise = true;
    uint32_t lastDisconnectMillis = 0;

    // Custom UUID Format (Randomly Generated):
    // XXXXXXXX-XXXX-4D91-B049-E2828E6DA1A0
    uint8_t uuidBuffer[16] = {
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00,
      0x4D, 0x91,
      0xB0, 0x49,
      0xE2, 0x82, 0x8E, 0x6D, 0xA1, 0xA0
    };
};

extern BluetoothManager GlobalBluetoothManager;