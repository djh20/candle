#include "bluetooth_metrics.h"
#include "bluetooth_manager.h"
#include "../../metric/metric_manager.h"
#include <BLEService.h>
#include <BLE2902.h>

#define UPDATE_INTERVAL 60

void BluetoothMetrics::begin()
{
  // Create bluetooth service for metrics.
  BLEService *metricsService = GlobalBluetoothManager.getServer()->createService(
    GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_S_METRICS)
  );

  MetricGroupInfo groupInfo;

  while (newGroup(groupInfo))
  {
    // Create characteristic for grouped metric data.
    BLECharacteristic *characteristic = new BLECharacteristic(
      GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_C_GROUPED_METRIC_DATA, groupInfo.id),
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );
    characteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
    metricsService->addCharacteristic(characteristic);
    characteristics[totalCharacteristics++] = characteristic;

    // Create descriptor for grouped metric info.
    BLEDescriptor *descriptor = new BLEDescriptor(
      GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_D_GROUPED_METRIC_INFO), 
      BLE_MAX_ATTRIBUTE_SIZE
    );
    descriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
    characteristic->addDescriptor(descriptor);

    BLEDescriptor *notifyDescriptor = new BLE2902();
    notifyDescriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
    characteristic->addDescriptor(notifyDescriptor);

    do 
    {
      log_i(
        "Assigned [%s] to group %u (c: %u, d: %u)", 
        groupInfo.metric->id, groupInfo.id, groupInfo.characteristicSize, groupInfo.descriptorSize
      );
    } 
    while (nextMetric(groupInfo));
  }

  // for (uint8_t i = 0; i < GlobalMetricManager.totalMetrics;)
  // {
  //   // Create characteristic for grouped metric data.
  //   BLECharacteristic *characteristic = new BLECharacteristic(
  //     GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_C_GROUPED_METRIC_DATA, totalCharacteristics),
  //     BLECharacteristic::PROPERTY_READ |
  //     BLECharacteristic::PROPERTY_NOTIFY
  //   );
  //   characteristic->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());

  //   uint8_t characteristicSize = 0;

  //   // Create descriptor for grouped metric info.
  //   BLEDescriptor *descriptor = new BLEDescriptor(
  //     GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_D_GROUPED_METRIC_INFO), 
  //     BLE_MAX_ATTRIBUTE_SIZE
  //   );
  //   descriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());

  //   // uint8_t descriptorBuffer[GROUPED_METRIC_INFO_LEN];
  //   memset(attributeBuffer, 0, sizeof(attributeBuffer));

  //   uint8_t descriptorSize = 0;
  
  //   for (; i < GlobalMetricManager.totalMetrics; i++) {
  //     Metric *metric = GlobalMetricManager.metrics[i];

  //     uint16_t newCharacteristicSize = characteristicSize;
  //     if (!metric->redacted) newCharacteristicSize += metric->getStateDataSize();

  //     uint16_t newDescriptorSize = descriptorSize + metric->getDescriptorDataSize();

  //     // Break the loop if we are going to exceed the max amount of data we can store.
  //     if (newCharacteristicSize > BLE_MAX_ATTRIBUTE_SIZE || newDescriptorSize > BLE_MAX_ATTRIBUTE_SIZE)
  //       break;

  //     // 0xFF indicates redacted metric.
  //     uint8_t dataIndex = metric->redacted ? 0xFF : characteristicSize;

  //     // Add descriptor data to buffer.
  //     metric->getDescriptorData(attributeBuffer, descriptorSize, dataIndex);

  //     characteristicSize = newCharacteristicSize;
  //   }

  //   descriptor->setValue(attributeBuffer, descriptorSize);

  //   characteristic->addDescriptor(descriptor);

  //   BLEDescriptor *notifyDescriptor = new BLE2902();
  //   notifyDescriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
  //   characteristic->addDescriptor(notifyDescriptor);

  //   metricsService->addCharacteristic(characteristic);
  //   characteristics[totalCharacteristics++] = characteristic;
  // }

  metricsService->start();
}

void BluetoothMetrics::loop()
{
  uint32_t now = millis();
  
  // if (now - lastUpdateMillis >= UPDATE_INTERVAL && GlobalBluetoothManager.isClientConnected())
  // {
  //   uint8_t characteristicIndex = 0;
  //   uint8_t descriptorSize = 0;

  //   for (uint8_t i = 0; i < GlobalMetricManager.totalMetrics;) {
  //     BLECharacteristic *characteristic = characteristics[characteristicIndex++];
  //     uint8_t characteristicSize = 0;
  //     bool updated = false;

  //     for (; i < GlobalMetricManager.totalMetrics; i++) {
  //       Metric *metric = GlobalMetricManager.metrics[i];
  //       if (metric->redacted) continue;
        
  //       uint16_t newCharacteristicSize = characteristicSize + metric->getStateDataSize();

  //       if (newCharacteristicSize > BLE_MAX_ATTRIBUTE_SIZE)
  //         break;

  //       metric->getStateData(attributeBuffer, characteristicSize);
  //       if (metric->lastUpdateMillis >= lastUpdateMillis) updated = true;
  //     }
      
  //     if (updated) 
  //     {
  //       characteristic->setValue(attributeBuffer, characteristicSize);
  //       characteristic->notify();
  //     }
  //   }

  //   lastUpdateMillis = now;
  // }
}

bool BluetoothMetrics::newGroup(MetricGroupInfo &groupInfo)
{
  while (true) 
  {
    // Only increment the ID if the current group has been used.
    if (groupInfo.descriptorSize > 0) groupInfo.id++;

    if (groupInfo.metricIndex >= GlobalMetricManager.totalMetrics-1)
    {
      groupInfo.metricType++;
      groupInfo.metricIndex = 0;
      groupInfo.metric = nullptr;
    }

    if (groupInfo.metricType >= METRIC_TYPE_ENUM_COUNT) break;

    groupInfo.descriptorSize = 0;
    groupInfo.characteristicSize = 0;
    
    if (nextMetric(groupInfo)) return true;
  }
  return false;
}

bool BluetoothMetrics::nextMetric(MetricGroupInfo &groupInfo)
{
  if (groupInfo.metric)
  {
    groupInfo.metricIndex++;
    groupInfo.metric = nullptr;
  }

  for (; groupInfo.metricIndex < GlobalMetricManager.totalMetrics; groupInfo.metricIndex++)
  {
    Metric *metric = GlobalMetricManager.metrics[groupInfo.metricIndex];
    uint8_t metricType = static_cast<uint8_t>(metric->type);

    if (metricType != groupInfo.metricType) continue;

    uint16_t newCharacteristicSize = groupInfo.characteristicSize;
    if (!metric->redacted) newCharacteristicSize += metric->getStateDataSize();

    uint16_t newDescriptorSize = groupInfo.descriptorSize + metric->getDescriptorDataSize();

    if (newCharacteristicSize > BLE_MAX_ATTRIBUTE_SIZE || newDescriptorSize > BLE_MAX_ATTRIBUTE_SIZE)
      break;
    
    groupInfo.metric = metric;
    groupInfo.descriptorSize = newDescriptorSize;
    groupInfo.characteristicSize = newCharacteristicSize;
    return true;
  }

  return false;

}

BluetoothMetrics GlobalBluetoothMetrics;