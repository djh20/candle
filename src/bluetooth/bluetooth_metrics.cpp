#include "bluetooth_metrics.h"
#include "bluetooth.h"
#include "../vehicle/vehicle_manager.h"
#include <BLEService.h>
#include <BLE2902.h>

#define UPDATE_INTERVAL 60U

static Vehicle *vehicle;
static BLECharacteristic *characteristics[16];
static uint8_t totalCharacteristics = 0;
static uint8_t characteristicValueBuffer[BLE_LEN_GROUPED_METRIC_DATA];
static uint32_t lastUpdateMillis = 0;

void BluetoothMetrics::begin()
{
  vehicle = VehicleManager::getVehicle();
  if (!vehicle)
  {
    log_w("Failed to create bluetooth metrics service - no vehicle instance");
    return;
  }

  // Create bluetooth service for metrics.
  BLEService *service = Bluetooth::getServer()->createService(
    Bluetooth::uuid(UUID_CUSTOM, BLE_SERVICE_METRICS)
  );


  for (uint8_t i = 0; i < vehicle->totalMetrics;)
  {
    // Create characteristic for grouped metric data.
    BLECharacteristic *characteristic = new BLECharacteristic(
      Bluetooth::uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_GROUPED_METRIC_DATA), // TODO: Add discriminator
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );
    characteristic->setAccessPermissions(Bluetooth::getAccessPermissions());

    uint8_t characteristicValueIndex = 0;

    // Create descriptor for grouped metric info.
    BLEDescriptor *descriptor = new BLEDescriptor(
      Bluetooth::uuid(UUID_CUSTOM, BLE_DESCRIPTOR_GROUPED_METRIC_INFO), 
      BLE_LEN_GROUPED_METRIC_INFO
    );
    descriptor->setAccessPermissions(Bluetooth::getAccessPermissions());

    uint8_t descriptorBuffer[BLE_LEN_GROUPED_METRIC_INFO];
    uint8_t descriptorBufferSize = 0;
  
    for (; i < vehicle->totalMetrics; i++) {
      Metric *metric = vehicle->metrics[i];

      // TODO: Somehow get data length without explicitly writing method for it.
      uint8_t characteristicValueLength = characteristicValueIndex + metric->getValueDataLength();

      // Break the loop if we are going to exceed the max amount of data that can be stored in one characteristic.
      if (characteristicValueLength > BLE_LEN_GROUPED_METRIC_DATA)
        break;

      metric->getDescriptorData(descriptorBuffer, descriptorBufferSize, characteristicValueIndex); // Add descriptor data to buffer.
      characteristicValueIndex = characteristicValueLength; // Update current value index.
    }

    descriptor->setValue(descriptorBuffer, descriptorBufferSize);

    characteristic->addDescriptor(descriptor);

    BLEDescriptor *notifyDescriptor = new BLE2902();
    notifyDescriptor->setAccessPermissions(Bluetooth::getAccessPermissions());
    characteristic->addDescriptor(notifyDescriptor);

    service->addCharacteristic(characteristic);
    characteristics[totalCharacteristics++] = characteristic;
  }

  service->start();
}

void BluetoothMetrics::loop()
{
  if (!vehicle) return;

  uint32_t now = millis();
  
  if (now - lastUpdateMillis >= UPDATE_INTERVAL && Bluetooth::getClientConnected())
  {
    uint8_t characteristicIndex = 0;

    for (uint8_t i = 0; i < vehicle->totalMetrics;) {
      BLECharacteristic *characteristic = characteristics[characteristicIndex];
      uint8_t characteristicValueIndex = 0;
      bool updated = false;

      for (; i < vehicle->totalMetrics; i++) {
        Metric *metric = vehicle->metrics[i];
        uint8_t characteristicValueLength = characteristicValueIndex + metric->getValueDataLength();

        if (characteristicValueLength > BLE_LEN_GROUPED_METRIC_DATA)
          break;
        
        metric->getValueData(characteristicValueBuffer, characteristicValueIndex);
        if (metric->lastUpdateMillis >= lastUpdateMillis) updated = true;
      }
      
      if (updated) {
        characteristic->setValue(characteristicValueBuffer, characteristicValueIndex);
        characteristic->notify();
      }
      characteristicIndex++;
    }

    lastUpdateMillis = now;
  }
}