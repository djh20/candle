#include "bluetooth_ota.h"
#include "bluetooth.h"
#include <BLEService.h>
#include <Update.h>

static char md5[33];

// Override function in esp32-hal-misc.c to disable automatic
// app verification after OTA. Not sure if this works...
extern "C" bool verifyRollbackLater() {
  return true;
}

class CommandCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    BluetoothOTA::processCommand(pCharacteristic->getData(), pCharacteristic->getLength());
  }
};

void BluetoothOTA::begin()
{
  BLEService *service = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_CUSTOM, BLE_SERVICE_OTA)
  );

  BLECharacteristic *commandCharacteristic = service->createCharacteristic(
    Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_OTA_COMMAND),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // TODO: Maybe OTA should only be available once a pin has been set.
  commandCharacteristic->setAccessPermissions(Bluetooth::getAccessPermissions());
  commandCharacteristic->setCallbacks(new CommandCallbacks());

  service->start();

  // Bluetooth::getServer()->getPeerMTU();

  // const esp_partition_t *partition = esp_ota_get_running_partition();
  // esp_ota_get_state_partition(partition);
}

void BluetoothOTA::processCommand(uint8_t *data, size_t length)
{
  if (length == 0) return;

  uint8_t id = data[0];

  if (id == 0x01 && length == 37) // Start OTA
  {
    // Calculate firmware size.
    uint32_t firmwareSize = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];

    log_i("Firmware Size: %u bytes", firmwareSize);
    
    // Ensure there is enough space for the update.
    bool ready = Update.begin(firmwareSize, U_FLASH);

    if (ready)
    {
      memset(md5, 0, sizeof(md5)); // Reset md5 buffer.
      memcpy(md5, data+5, sizeof(md5)-1); // Copy data to md5 buffer (minus one for null terminator).

      log_i("Firmware MD5: %s", md5);
      Update.setMD5(md5);
    }
  }
  else
  {
    log_e("Invalid command received");
  }
}