#pragma once

#include <BLEUUID.h>
#include <BLEServer.h>

#define UUID_STANDARD false
#define UUID_CUSTOM true

#define BLE_MODE_OPEN 0
#define BLE_MODE_ENCRYPTED 1

#define BLE_STD_SERVICE_DEVICE_INFO 0x180A
#define BLE_STD_CHARACTERISTIC_MODEL_NUMBER 0x2A24
#define BLE_STD_CHARACTERISTIC_FIRMWARE_VERSION 0x2A26

#define BLE_SERVICE_OTA 0x0000
#define BLE_CHARACTERISTIC_OTA_COMMAND 0x0001
#define BLE_CHARACTERISTIC_OTA_DATA 0x0002

#define BLE_SERVICE_DEBUG 0x0100
#define BLE_CHARACTERISTIC_DEBUG_COMMAND 0x0101
#define BLE_CHARACTERISTIC_DEBUG_LOG 0x0102

#define BLE_SERVICE_CONFIG 0x0200
#define BLE_CHARACTERISTIC_CONFIG_VEHICLE_ID 0x0201
#define BLE_CHARACTERISTIC_CONFIG_BLE_MODE 0x0202
#define BLE_CHARACTERISTIC_CONFIG_BLUETOOTH_PIN 0x0203

#define BLE_SERVICE_VEHICLE_CATALOG 0x0300
#define BLE_CHARACTERISTIC_SUPPORTED_VEHICLE 0x0301

#define BLE_SERVICE_METRICS 0x0400
#define BLE_CHARACTERISTIC_GROUPED_METRIC_DATA 0x0401
#define BLE_DESCRIPTOR_GROUPED_METRIC_INFO 0x0402

class BluetoothManager: public BLEServerCallbacks
{
  public:
    void begin();
    void loop();

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    void setCanAdvertise(bool value);
    BLEServer *getServer();
    bool getClientConnected();
    BLEUUID uuid(bool custom, uint16_t id, uint16_t discriminator = 0x0000);
    esp_gatt_perm_t getAccessPermissions();
  
  private:
    BLEServer *server;
    bool clientConnected = false;
    bool advertising = false;
    bool canAdvertise = true;
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