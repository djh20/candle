#include <Arduino.h>
#include "command_callbacks.h"
#include "../../utils/logger.h"

CommandCallbacks::CommandCallbacks(Vehicle *vehicle) : BLECharacteristicCallbacks() {
  this->vehicle = vehicle;
}

void CommandCallbacks::onWrite(BLECharacteristic *pCharacteristic) {
  uint8_t *data = pCharacteristic->getData();
  uint16_t metricId = (data[0] << 8) | data[1];

  for (uint8_t i = 0; i < vehicle->totalMetrics; i++) {
    Metric *metric = vehicle->metrics[i];
    if (metric->id == metricId) {
      metric->setValueFromRawData(&data[2]);
      break;
    }
  }
}
