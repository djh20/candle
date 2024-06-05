#include "bluetooth_vehicle.h"
#include "bluetooth.h"
#include "../vehicle/vehicle_manager.h"
#include <BLEService.h>
#include <BLE2902.h>

#define UPDATE_INTERVAL 60U

void BluetoothVehicle::begin()
{
  vehicle = GlobalVehicleManager.getVehicle();
  if (!vehicle)
  {
    log_w("Failed to create bluetooth metrics service - no vehicle instance");
    return;
  }

  // Create bluetooth service for metrics.
  BLEService *metricsService = GlobalBluetooth.getServer()->createService(
    GlobalBluetooth.uuid(UUID_CUSTOM, BLE_SERVICE_METRICS)
  );

  for (uint8_t i = 0; i < vehicle->totalMetrics;)
  {
    // Create characteristic for grouped metric data.
    BLECharacteristic *characteristic = new BLECharacteristic(
      GlobalBluetooth.uuid(UUID_CUSTOM, BLE_CHARACTERISTIC_GROUPED_METRIC_DATA, totalCharacteristics),
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );
    characteristic->setAccessPermissions(GlobalBluetooth.getAccessPermissions());

    uint8_t characteristicValueIndex = 0;

    // Create descriptor for grouped metric info.
    BLEDescriptor *descriptor = new BLEDescriptor(
      GlobalBluetooth.uuid(UUID_CUSTOM, BLE_DESCRIPTOR_GROUPED_METRIC_INFO), 
      GROUPED_METRIC_INFO_LEN
    );
    descriptor->setAccessPermissions(GlobalBluetooth.getAccessPermissions());

    uint8_t descriptorBuffer[GROUPED_METRIC_INFO_LEN];
    uint8_t descriptorBufferSize = 0;
  
    for (; i < vehicle->totalMetrics; i++) {
      Metric *metric = vehicle->metrics[i];

      // TODO: Somehow get data length without explicitly writing method for it.
      uint8_t characteristicValueLength = characteristicValueIndex + metric->getValueDataLength();

      // Break the loop if we are going to exceed the max amount of data that can be stored in one characteristic.
      if (characteristicValueLength > GROUPED_METRIC_DATA_LEN)
        break;

      metric->getDescriptorData(descriptorBuffer, descriptorBufferSize, characteristicValueIndex); // Add descriptor data to buffer.
      characteristicValueIndex = characteristicValueLength; // Update current value index.
    }

    descriptor->setValue(descriptorBuffer, descriptorBufferSize);

    characteristic->addDescriptor(descriptor);

    BLEDescriptor *notifyDescriptor = new BLE2902();
    notifyDescriptor->setAccessPermissions(GlobalBluetooth.getAccessPermissions());
    characteristic->addDescriptor(notifyDescriptor);

    metricsService->addCharacteristic(characteristic);
    characteristics[totalCharacteristics++] = characteristic;
  }

  metricsService->start();
}

void BluetoothVehicle::loop()
{
  if (!vehicle) return;

  GlobalBluetooth.setCanAdvertise(!vehicle->awake->initialized || vehicle->awake->value);

  uint32_t now = millis();
  
  if (now - lastUpdateMillis >= UPDATE_INTERVAL && GlobalBluetooth.getClientConnected())
  {
    uint8_t characteristicIndex = 0;

    for (uint8_t i = 0; i < vehicle->totalMetrics;) {
      BLECharacteristic *characteristic = characteristics[characteristicIndex];
      uint8_t characteristicValueIndex = 0;
      bool updated = false;

      for (; i < vehicle->totalMetrics; i++) {
        Metric *metric = vehicle->metrics[i];
        uint8_t characteristicValueLength = characteristicValueIndex + metric->getValueDataLength();

        if (characteristicValueLength > GROUPED_METRIC_DATA_LEN)
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

BluetoothVehicle GlobalBluetoothVehicle;