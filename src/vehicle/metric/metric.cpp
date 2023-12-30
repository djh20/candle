#include "metric.h"
#include <BLE2902.h>

Metric::Metric(uint16_t id, Unit unit) 
{
  this->id = id;

  bleCharacteristic = new BLECharacteristic(
    BLEUUID(id),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  bleDescriptor = new BLEDescriptor(BLEUUID((uint16_t) 0x0000));

  bleCharacteristic->addDescriptor(new BLE2902());
  bleCharacteristic->addDescriptor(bleDescriptor);
}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::setValueFromString(String str) {}

MetricInt::MetricInt(uint16_t id, Unit unit, int32_t minValue, int32_t maxValue) : Metric(id, unit) 
{
  this->minValue = minValue;
  this->maxValue = maxValue;

  uint8_t descriptorData[2] = {MetricType::Int, unit};
  bleDescriptor->setValue(descriptorData, 2);
}

void MetricInt::setValue(int32_t newValue, bool force)
{
  if (newValue == value && initialized) return;
  if (!force && (newValue < minValue || newValue > maxValue)) return;

  initialized = true;
  value = newValue;

  bleCharacteristic->setValue(value);
  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricInt::setValueFromString(String str) {
  setValue(str.toInt(), true);
}

MetricFloat::MetricFloat(uint16_t id, Unit unit, Precision precision, float minValue, float maxValue) : Metric(id, unit) 
{
  this->minValue = minValue;
  this->maxValue = maxValue;
  this->precision = precision;

  uint8_t descriptorData[3] = {MetricType::Float, unit, precision};
  bleDescriptor->setValue(descriptorData, 3);
}

void MetricFloat::setValue(float newValue, bool force)
{
  if (newValue == value && initialized) return;
  if (!force && (newValue < minValue || newValue > maxValue)) return;

  initialized = true;
  value = newValue;

  int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
  bleCharacteristic->setValue(convertedValue);

  lastUpdateMillis = millis();
  if (updateHandler) updateHandler();
}

void MetricFloat::setValueFromString(String str) {
  setValue(str.toFloat(), true);
}
