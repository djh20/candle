#include <Arduino.h>
#include "metric_ble_callbacks.h"
#include "../../utils/logger.h"

MetricBLECallbacks::MetricBLECallbacks(Metric *metric) : BLECharacteristicCallbacks() {
  this->metric = metric;
}

void MetricBLECallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  uint8_t *data = pCharacteristic->getData();
  metric->setValueFromRawData(data);
}
