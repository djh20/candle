#include "metric_int.h"

IntMetric::IntMetric(const char *domain, const char *localId, MetricType type, Unit unit) 
  : Metric(domain, localId, type, MetricDataType::Int, unit) {}

void IntMetric::loadValue()
{
  setValue(prefs.getInt(localId));
}

void IntMetric::saveValue()
{
  prefs.putInt(localId, value);
}

void IntMetric::setValue(int32_t newValue)
{
  if (newValue == value && valid) return;

  valid = true;
  value = newValue;

  markAsUpdated();
  save();
}

void IntMetric::setValueFromString(const char *str)
{
  setValue(strtol(str, nullptr, 0));
}

void IntMetric::getValueAsString(char *str)
{
  sprintf(str, "%d", value);
}

void IntMetric::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  buffer[bufferIndex++] = valid;
  buffer[bufferIndex++] = value >> 24;
  buffer[bufferIndex++] = value >> 16;
  buffer[bufferIndex++] = value >> 8;
  buffer[bufferIndex++] = value;
}

uint8_t IntMetric::getValueDataLength() {
  return 5;
}