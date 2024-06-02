#include "bluetooth_ota.h"
#include "bluetooth.h"
#include <BLEService.h>
#include <BLE2902.h>
#include <Update.h>
#include <esp_ota_ops.h>

#define STATUS_CODE_SUCCESS 0U
#define STATUS_CODE_ERROR 1U

#define VERIFY_DELAY 3000U
#define RESTART_DELAY 500U

// Stores the MD5 hash string of incoming firmware.
// Extra byte for null terminator.
static char hashStr[ESP_ROM_MD5_DIGEST_LEN * 2 + 1];

static uint8_t responseData[2] = {0xFF, 0x00};
static BLECharacteristic *commandCharacteristic;
static BLECharacteristic *dataCharacteristic;
static bool updateFinished = false;
static float updateProgressPercent;
static uint32_t updateFinishMillis = 0;
static esp_ota_img_states_t partitionState = ESP_OTA_IMG_INVALID;

// Override function in esp32-hal-misc.c to disable automatic
// app verification after OTA. Not sure if this works...
extern "C" bool verifyRollbackLater() {
  return true;
}

class CharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    if (pCharacteristic == commandCharacteristic)
    {
      BluetoothOTA::processCommand(pCharacteristic->getData(), pCharacteristic->getLength());
    }
    else if (pCharacteristic == dataCharacteristic)
    {
      BluetoothOTA::writeFirmware(pCharacteristic->getData(), pCharacteristic->getLength());
    }
  }
};

static void sendResponse(uint8_t statusCode)
{
  responseData[1] = statusCode;
  commandCharacteristic->setValue(responseData, sizeof(responseData));
  commandCharacteristic->notify();
}

void BluetoothOTA::begin()
{
  const esp_partition_t *running = esp_ota_get_running_partition();
  esp_ota_get_state_partition(running, &partitionState);

  BLEService *service = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_CUSTOM, BLE_SERVICE_OTA)
  );

  commandCharacteristic = service->createCharacteristic(
    Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_OTA_COMMAND),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // TODO: Maybe OTA should only be available once a pin has been set.
  commandCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());
  commandCharacteristic->setCallbacks(new CharacteristicCallbacks());

  BLEDescriptor *notifyDescriptor = new BLE2902();
  notifyDescriptor->setAccessPermissions(Bluetooth::getAccessPermissions());
  commandCharacteristic->addDescriptor(notifyDescriptor);

  dataCharacteristic = service->createCharacteristic(
    Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_OTA_DATA),
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR |
    BLECharacteristic::PROPERTY_INDICATE
  );

  dataCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());
  dataCharacteristic->setCallbacks(new CharacteristicCallbacks());

  service->start();
}

void BluetoothOTA::processCommand(uint8_t *data, size_t length)
{
  if (length == 0) return;

  uint8_t id = data[0];

  if (id == 0x01 && length == 1 + 4 + ESP_ROM_MD5_DIGEST_LEN) // Start OTA
  {
    log_i("Starting OTA update");
    
    // Calculate firmware size.
    uint32_t firmwareSize = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];

    log_i("Firmware Size: %u bytes", firmwareSize);
    
    // Ensure there is enough space for the update.
    bool success = Update.begin(firmwareSize, U_FLASH);

    if (success)
    {
      // We need to convert the provided MD5 hash (16 bytes) to a string (32 bytes)
      // because the Update library requires a string.

      memset(hashStr, 0, sizeof(hashStr)); // Clear hash string.

      for (uint8_t i = 0; i < ESP_ROM_MD5_DIGEST_LEN; i++) 
      {
        sprintf(hashStr + (i * 2), "%02x", data[i+5]);
      }

      log_i("Firmware MD5 Hash: %s", hashStr);
      success = Update.setMD5(hashStr);
      if (success)
      {
        sendResponse(STATUS_CODE_SUCCESS);
      }
      else
      {
        log_e("Failed to start OTA update - MD5 hash invalid");
        sendResponse(STATUS_CODE_ERROR);
      }
    }
    else
    {
      log_e("Failed to start OTA update - not enough space");
      sendResponse(STATUS_CODE_ERROR);
    }
  }
  // else if (id == 0x02 && length >= 2) 
  // {
  //   if (Update.write(data+1, length-1) > 0)
  //   {
  //     sendResponse(STATUS_CODE_SUCCESS);
  //   }
  //   else
  //   {
  //     log_e("Failed to write OTA data");
  //     sendResponse(STATUS_CODE_ERROR);
  //   }
  // }
  else if (id == 0x02 && length == 1)
  {
    if(Update.end())
    {
      log_i("OTA update completed");
      updateFinishMillis = millis();
      updateFinished = true;
      sendResponse(STATUS_CODE_SUCCESS);
    }
    else
    {
      log_e("Failed to complete OTA update");
      sendResponse(STATUS_CODE_ERROR);
    }
  }
  else
  {
    log_e("Invalid command received");
    sendResponse(STATUS_CODE_ERROR);
  }
}

void BluetoothOTA::writeFirmware(uint8_t *data, size_t length)
{
  if (length == 0) return;

  if (Update.write(data, length) > 0)
  {
    updateProgressPercent = (Update.progress() / (float)Update.size()) * 100;
    log_i("Firmware update progress: %.2f%% (%zu / %zu bytes)", updateProgressPercent, Update.progress(), Update.size());
  }
  else
  {
    log_e("Failed to write OTA data");
  }
}

void BluetoothOTA::loop()
{
  uint32_t now = millis();

  if (partitionState == ESP_OTA_IMG_PENDING_VERIFY && now >= VERIFY_DELAY)
  {
    log_i("Verified app partition");
    esp_ota_mark_app_valid_cancel_rollback();
    partitionState = ESP_OTA_IMG_VALID;
  }

  if (updateFinished && now - updateFinishMillis >= RESTART_DELAY)
  {
    updateFinished = false;
    ESP.restart();
  }
}