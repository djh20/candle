#include "bluetooth_ota.h"
#include "bluetooth.h"
#include <BLEService.h>
// #include <Update.h>
// #include <esp_ota_ops.h>

void BluetoothOTA::begin()
{
  BLEService *service = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_CUSTOM, BLE_SERVICE_OTA)
  );

  service->start();

  //Update.begin();

  // const esp_partition_t *partition = esp_ota_get_running_partition();
  // esp_ota_get_state_partition(partition);
}
