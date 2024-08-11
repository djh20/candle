#include "bluetooth_metrics.h"
#include "bluetooth_manager.h"
#include "../../metric/metric_manager.h"
#include <BLEService.h>
#include <BLE2902.h>

#define UPDATE_INTERVAL 60

void BluetoothMetrics::begin()
{
  groupAssignments = new uint8_t[GlobalMetricManager.totalMetrics];

  // Create bluetooth service for metrics.
  BLEService *metricsService = GlobalBluetoothManager.getServer()->createService(
    GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_S_METRICS), 30
  );

  // The following code splits metrics into groups to optimize Bluetooth performance. 
  // Additionally, it also ensures parameters and statistics are in separate groups. 
  // This prevents frequent updates of statistics from causing unnecessary transmission 
  // of parameters.

  // Loop through all possible metric types.
  for (uint8_t t = 0; t <= GlobalMetricManager.maxMetricType; t++)
  {
    for (uint8_t m = 0; m < GlobalMetricManager.totalMetrics;) 
    {
      uint16_t characteristicSize = 0, descriptorSize = 0;
      attributeBufferSize = 0;

      // Inner loop to group metrics by type and check if they fit within the attribute buffer.
      for (; m < GlobalMetricManager.totalMetrics; m++)
      {
        Metric *metric = GlobalMetricManager.metrics[m];
        uint8_t metricType = static_cast<uint8_t>(metric->type);

        if (metricType != t) continue;

        // Calculate the new size of the characteristic if this metric is added.
        uint16_t newCharacteristicSize = characteristicSize;
        if (!metric->redacted) newCharacteristicSize += metric->getStateDataSize();

        // Calculate the new size of the descriptor if this metric is added.
        uint16_t newDescriptorSize = descriptorSize + metric->getDescriptorDataSize();
        
        // If adding this metric exceeds the max size, exit the loop to start a new group.
        if (newCharacteristicSize > sizeof(attributeBuffer) || newDescriptorSize > sizeof(attributeBuffer))
          break;

        uint8_t dataIndex = metric->redacted ? 0xFF : characteristicSize;
        metric->getDescriptorData(attributeBuffer, attributeBufferSize, dataIndex);

        // Assign the current metric to the current group.
        groupAssignments[m] = totalGroups;

        // Update the characteristic and descriptor sizes with the new metric included.
        characteristicSize = newCharacteristicSize;
        descriptorSize = newDescriptorSize;

        log_i(
          "Assigned [%s] to group %u (c: %u, d: %u)", 
          metric->id, totalGroups, characteristicSize, descriptorSize
        );
      }

      // If any metrics were added to this group, create BLE characteristic and descriptor.
      if (characteristicSize > 0 || descriptorSize > 0)
      {
        BLECharacteristic *characteristic = new BLECharacteristic(
          GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_C_GROUPED_METRIC_DATA, totalGroups),
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY
        );
        characteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
        metricsService->addCharacteristic(characteristic);
        characteristics[totalGroups] = characteristic;

        BLEDescriptor *descriptor = new BLEDescriptor(
          GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_D_GROUPED_METRIC_INFO), 
          sizeof(attributeBuffer)
        );
        descriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
        descriptor->setValue(attributeBuffer, attributeBufferSize);
        characteristic->addDescriptor(descriptor);

        BLEDescriptor *notifyDescriptor = new BLE2902();
        notifyDescriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
        characteristic->addDescriptor(notifyDescriptor);

        totalGroups++;
      }
    }
  }

  metricsService->start();
}

void BluetoothMetrics::loop()
{
  uint32_t now = millis();
  
  if (now - lastTransmissionMillis >= UPDATE_INTERVAL)
  {
    for (uint8_t g = 0; g < totalGroups; g++)
    {
      bool shouldTransmit = false;
      attributeBufferSize = 0;

      for (uint8_t m = 0; m < GlobalMetricManager.totalMetrics; m++)
      {
        // Skip metrics that are not assigned to the current group.
        if (groupAssignments[m] != g) continue;
        
        Metric *metric = GlobalMetricManager.metrics[m];

        // Skip redacted metrics.
        if (metric->redacted) continue;

        // Retrieve the metric's state data and append it to the attribute buffer.
        metric->getStateData(attributeBuffer, attributeBufferSize);
        
        // If the metric was updated since the last transmission, set the shouldTransmit flag.
        if (metric->lastUpdateMillis >= lastTransmissionMillis)
        {
          shouldTransmit = true;
        }
      }

      if (shouldTransmit)
      {
        BLECharacteristic *characteristic = characteristics[g];
        characteristic->setValue(attributeBuffer, attributeBufferSize);

        if (GlobalBluetoothManager.isClientConnected()) characteristic->notify();
      }
    }

    lastTransmissionMillis = now;
  }
}

BluetoothMetrics GlobalBluetoothMetrics;