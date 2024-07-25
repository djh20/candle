#pragma once

#include <Arduino.h>
#include <Update.h>
#include <BLECharacteristic.h>
#include <esp_ota_ops.h>

class BluetoothOTA: public BLECharacteristicCallbacks
{
  public:
    void begin();
    void loop();
  
  private:
    void processCommand(uint8_t *data, size_t length);
    void writeFirmware(uint8_t *data, size_t length);
    void sendResponse(uint8_t statusCode);

    void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param);

    // Stores the MD5 hash string of incoming firmware.
    // Extra byte for null terminator.
    char hashStr[ESP_ROM_MD5_DIGEST_LEN * 2 + 1];
    uint8_t responseData[2] = {0xFF, 0x00};
    BLECharacteristic *commandCharacteristic;
    BLECharacteristic *dataCharacteristic;
    bool updateFinished = false;
    float updateProgressPercent;
    uint32_t updateFinishMillis = 0;
    esp_ota_img_states_t partitionState = ESP_OTA_IMG_INVALID;
};

extern BluetoothOTA GlobalBluetoothOTA;