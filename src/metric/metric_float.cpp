#include "metric_float.h"

FloatMetric::FloatMetric(
  const char *domain, const char *localId, MetricType type, Unit unit, Precision precision
) : Metric(domain, localId, type, MetricDataType::Float, unit)
{
  this->precision = precision;
}

void FloatMetric::loadValue()
{
  setValue(prefs.getFloat(localId));
}

void FloatMetric::saveValue()
{
  prefs.putFloat(localId, value);
}

void FloatMetric::setValue(float newValue)
{
  if (newValue == value && valid) return;

  valid = true;
  value = newValue;

  markAsUpdated();
  save();
}

void FloatMetric::setValueFromString(const char *str)
{
  setValue(strtof(str, nullptr));
}

void FloatMetric::getValueAsString(char *str)
{
  sprintf(str, "%f", value);
}

void FloatMetric::getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex) {
  Metric::getDescriptorData(buffer, bufferIndex, valueDataIndex);

  buffer[bufferIndex++] = static_cast<uint8_t>(precision);
}

void FloatMetric::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
  buffer[bufferIndex++] = valid;
  buffer[bufferIndex++] = convertedValue >> 24;
  buffer[bufferIndex++] = convertedValue >> 16;
  buffer[bufferIndex++] = convertedValue >> 8;
  buffer[bufferIndex++] = convertedValue;
}

uint8_t FloatMetric::getValueDataLength() {
  return 5;
}