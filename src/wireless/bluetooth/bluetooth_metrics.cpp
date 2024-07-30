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
    characteristics[groupInfo.id] = characteristic;

    // Create descriptor for grouped metric info.
    BLEDescriptor *descriptor = new BLEDescriptor(
      GlobalBluetoothManager.uuid(UUID_CUSTOM, BLE_D_GROUPED_METRIC_INFO), 
      sizeof(attributeBuffer)
    );
    descriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
    characteristic->addDescriptor(descriptor);

    BLEDescriptor *notifyDescriptor = new BLE2902();
    notifyDescriptor->setAccessPermissions(GlobalBluetoothManager.getAccessPermissions());
    characteristic->addDescriptor(notifyDescriptor);

    attributeBufferSize = 0;

    do 
    {
      log_i(
        "Assigned [%s] to group %u (c: %u, d: %u)", 
        groupInfo.metric->id, groupInfo.id, groupInfo.characteristicSize, groupInfo.descriptorSize
      );

      // 0xFF indicates redacted metric.
      uint8_t dataIndex = groupInfo.metric->redacted ? 0xFF : groupInfo.characteristicPos;

      groupInfo.metric->getDescriptorData(attributeBuffer, attributeBufferSize, dataIndex);
    } 
    while (nextMetric(groupInfo));

    descriptor->setValue(attributeBuffer, attributeBufferSize);
  }

  metricsService->start();
}

void BluetoothMetrics::loop()
{
  uint32_t now = millis();
  
  if (now - lastUpdateMillis >= UPDATE_INTERVAL)
  {
    MetricGroupInfo groupInfo;
    
    while (newGroup(groupInfo))
    {
      bool shouldUpdate = false;
      attributeBufferSize = 0;

      do 
      {
        // Skip metrics with no characteristic data (most likely due to redacted metric).
        if (groupInfo.characteristicPos == groupInfo.characteristicSize) continue;

        groupInfo.metric->getStateData(attributeBuffer, attributeBufferSize);
    
        if (groupInfo.metric->lastUpdateMillis >= lastUpdateMillis) shouldUpdate = true;
      } 
      while (nextMetric(groupInfo));
      
      // Only update the characteristic if at least one of the metrics has changed.
      if (shouldUpdate)
      {
        BLECharacteristic *characteristic = characteristics[groupInfo.id];
        characteristic->setValue(attributeBuffer, attributeBufferSize);

        if (GlobalBluetoothManager.isClientConnected()) characteristic->notify();
      }
    }

    lastUpdateMillis = millis();
  }
}

/*
  Splits metrics into groups to optimize Bluetooth performance, ensuring parameters 
  and statistics are in separate groups. This prevents frequent updates of statistics 
  from causing unnecessary data transmission of parameters.
*/
bool BluetoothMetrics::newGroup(MetricGroupInfo &groupInfo)
{
  while (true) 
  {
    // Only increment the ID if the current group has been used.
    if (groupInfo.descriptorSize > 0) groupInfo.id++;

    // Move to the next metric type after we've gone through all the metrics.
    if (groupInfo.metricIndex >= GlobalMetricManager.totalMetrics-1)
    {
      groupInfo.metricType++;
      groupInfo.metricIndex = 0;
      groupInfo.metric = nullptr;
    }

    // Exit if all metric types have been processed.
    if (groupInfo.metricType >= METRIC_TYPE_ENUM_COUNT) break;

    // Reset descriptor and characteristic position & sizes.
    groupInfo.descriptorPos = 0;
    groupInfo.characteristicPos = 0;
    groupInfo.descriptorSize = 0;
    groupInfo.characteristicSize = 0;
    
    // Try to find the first metric for this group and return true if successful.
    if (nextMetric(groupInfo)) return true;
  }
  return false;
}

/*
  Finds and adds the next metric of the current type to the group, while ensuring 
  the characteristic and descriptor sizes do not exceed Bluetooth limits.
*/
bool BluetoothMetrics::nextMetric(MetricGroupInfo &groupInfo)
{
  // Prepare for next metric if a metric is already assigned.
  if (groupInfo.metric)
  {
    groupInfo.metricIndex++;
    groupInfo.metric = nullptr;
    groupInfo.characteristicPos = groupInfo.characteristicSize;
    groupInfo.descriptorPos = groupInfo.descriptorSize;
  }

  // Iterate over the remaining metrics.
  for (; groupInfo.metricIndex < GlobalMetricManager.totalMetrics; groupInfo.metricIndex++)
  {
    Metric *metric = GlobalMetricManager.metrics[groupInfo.metricIndex];
    uint8_t metricType = static_cast<uint8_t>(metric->type);

    // Skip metrics that do not match the current metric type.
    if (metricType != groupInfo.metricType) continue;

    // Calculate new sizes for characteristic and descriptor.
    uint16_t newCharacteristicSize = groupInfo.characteristicSize;
    if (!metric->redacted) newCharacteristicSize += metric->getStateDataSize();

    uint16_t newDescriptorSize = groupInfo.descriptorSize + metric->getDescriptorDataSize();

    // Check if the new sizes exceed the maximum attribute size.
    if (newCharacteristicSize > sizeof(attributeBuffer) || newDescriptorSize > sizeof(attributeBuffer))
      break;
    
    // Assign the metric to the group and update sizes.
    groupInfo.metric = metric;
    groupInfo.descriptorSize = newDescriptorSize;
    groupInfo.characteristicSize = newCharacteristicSize;
    return true;
  }

  return false;
}

BluetoothMetrics GlobalBluetoothMetrics;