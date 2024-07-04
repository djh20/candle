#include "metric_string.h"

StringMetric::StringMetric(
  const char *domain, const char *localId, MetricType type, uint8_t size
) : Metric(domain, localId, type, MetricDataType::String, Unit::None) 
{
  value = new char[size]();
  valueSize = size;
}

void StringMetric::loadValue()
{
  prefs.getString(localId, value, valueSize);
  valid = true;
  markAsUpdated();
}

void StringMetric::saveValue()
{
  prefs.putString(localId, value);
}

void StringMetric::setValue(const char *newValue)
{
  if (strcmp(value, newValue) == 0 && valid) return;

  strcpy(value, newValue);
  valid = true;

  markAsUpdated();
  save();
}

void StringMetric::setValueFromString(const char *str)
{
  setValue(str);
}

void StringMetric::getValueAsString(char *str)
{
  strcpy(str, value);
}

void StringMetric::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  // TODO: Implement
}

uint8_t StringMetric::getValueDataLength() {
  // TODO: Implement
  return 0;
}