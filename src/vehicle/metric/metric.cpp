#include "metric.h"
#include <BLE2902.h>

Metric::Metric(uint16_t id, Unit unit) 
{
  this->id = id;
  this->unit = unit;

  // bleCharacteristic = new BLECharacteristic(
  //   BLEUUID(id),
  //   BLECharacteristic::PROPERTY_READ |
  //   BLECharacteristic::PROPERTY_WRITE |
  //   BLECharacteristic::PROPERTY_NOTIFY
  // );

  // bleDescriptor = new BLEDescriptor(BLEUUID((uint16_t) 0x0000));
  // bleCharacteristic->addDescriptor(bleDescriptor);

  // bleCharacteristic->addDescriptor(new BLE2902());
  
  // bleCharacteristic->setCallbacks(new MetricBLECallbacks(this));
}

void Metric::onUpdate(std::function<void()> handler)
{
  updateHandler = handler;
}

void Metric::setValueFromRawData(uint8_t *data) {}
void Metric::setValueFromString(String str) {}
uint8_t Metric::getDataSize() {}
void Metric::getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex) {}
void Metric::getValueData(uint8_t buffer[], uint8_t &bufferIndex) {}

MetricInt::MetricInt(uint16_t id, Unit unit) : Metric(id, unit) {}

void MetricInt::setValue(int32_t newValue)
{
  if (newValue == value && initialized) return;

  initialized = true;
  value = newValue;

  lastUpdateMillis = millis();

  if (updateHandler) updateHandler();
}

void MetricInt::setValueFromRawData(uint8_t *data) {
  //setValue(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
  setValue((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);
}

void MetricInt::setValueFromString(String str) {
  setValue(str.toInt());
}

uint8_t MetricInt::getDataSize() {
  return 5;
}

void MetricInt::getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex) {
  buffer[bufferIndex++] = id >> 8;
  buffer[bufferIndex++] = id;
  buffer[bufferIndex++] = valueDataIndex;
  buffer[bufferIndex++] = MetricType::Int;
  buffer[bufferIndex++] = unit;
}

void MetricInt::getValueData(uint8_t buffer[], uint8_t &bufferIndex) {
  buffer[bufferIndex++] = initialized;
  buffer[bufferIndex++] = value >> 24;
  buffer[bufferIndex++] = value >> 16;
  buffer[bufferIndex++] = value >> 8;
  buffer[bufferIndex++] = value;
}

MetricFloat::MetricFloat(uint16_t id, Unit unit, Precision precision) : Metric(id, unit) 
{
  this->precision = precision;
}

void MetricFloat::setValue(float newValue)
{
  if (newValue == value && initialized) return;

  initialized = true;
  value = newValue;

  lastUpdateMillis = millis();

  if (updateHandler) updateHandler();
}

void MetricFloat::setValueFromRawData(uint8_t *data) {
  int32_t intValue = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  setValue(intValue / pow(10, (uint8_t)precision));
}

void MetricFloat::setValueFromString(String str) {
  setValue(str.toFloat());
}

uint8_t MetricFloat::getDataSize() {
  return 5;
}

void MetricFloat::getDescriptorData(uint8_t buffer[], uint8_t &bufferIndex, uint8_t valueDataIndex) {
  buffer[bufferIndex++] = id >> 8;
  buffer[bufferIndex++] = id;
  buffer[bufferIndex++] = valueDataIndex;
  buffer[bufferIndex++] = MetricType::Float;
  buffer[bufferIndex++] = unit;
  buffer[bufferIndex++] = precision;
}

void MetricFloat::getValueData(uint8_t buffer[], uint8_t &bufferIndex) {
  int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
  buffer[bufferIndex++] = initialized;
  buffer[bufferIndex++] = convertedValue >> 24;
  buffer[bufferIndex++] = convertedValue >> 16;
  buffer[bufferIndex++] = convertedValue >> 8;
  buffer[bufferIndex++] = convertedValue;
}