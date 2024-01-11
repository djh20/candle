#include "metric.h"
#include "metric_ble_callbacks.h"
#include <BLE2902.h>

Metric::Metric(uint16_t id, Unit unit) 
{
  this->id = id;

  bleCharacteristic = new BLECharacteristic(
    BLEUUID(id),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  bleDescriptor = new BLEDescriptor(BLEUUID((uint16_t) 0x0000));
  bleCharacteristic->addDescriptor(bleDescriptor);

  bleCharacteristic->addDescriptor(new BLE2902());
  
  bleCharacteristic->setCallbacks(new MetricBLECallbacks(this));
}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::setValueFromRawData(uint8_t *data) {}
void Metric::setValueFromString(String str) {}

MetricInt::MetricInt(uint16_t id, Unit unit) : Metric(id, unit) 
{
  uint8_t descriptorData[2] = {MetricType::Int, unit};
  bleDescriptor->setValue(descriptorData, 2);
}

void MetricInt::setValue(int32_t newValue, bool publish)
{
  if (newValue == value && initialized) return;

  initialized = true;
  value = newValue;

  if (publish) {
    bleCharacteristic->setValue(value);
    lastUpdateMillis = millis();
  }

  if (updateHandler) updateHandler();
}

void MetricInt::setValueFromRawData(uint8_t *data) {
  setValue(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

void MetricInt::setValueFromString(String str) {
  setValue(str.toInt(), true);
}

MetricFloat::MetricFloat(uint16_t id, Unit unit, Precision precision) : Metric(id, unit) 
{
  this->precision = precision;

  uint8_t descriptorData[3] = {MetricType::Float, unit, precision};
  bleDescriptor->setValue(descriptorData, 3);
}

void MetricFloat::setValue(float newValue, bool publish)
{
  if (newValue == value && initialized) return;

  initialized = true;
  value = newValue;

  if (publish) {
    int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
    bleCharacteristic->setValue(convertedValue);

    lastUpdateMillis = millis();
  }

  if (updateHandler) updateHandler();
}

void MetricFloat::setValueFromRawData(uint8_t *data) {
  int32_t intValue = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
  setValue(intValue / pow(10, (uint8_t)precision));
}

void MetricFloat::setValueFromString(String str) {
  setValue(str.toFloat(), true);
}
