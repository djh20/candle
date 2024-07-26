#include "metric_float.h"

template <uint8_t E>
FloatMetric<E>::FloatMetric(
  const char *domain, const char *localId, MetricType type, Unit unit, Precision precision
) : Metric(domain, localId, type, MetricDataType::Float, unit, E)
{
  this->precision = precision;
}

template <uint8_t E>
void FloatMetric<E>::loadState()
{
  prefs.getBytes(localId, state, sizeof(state));
}

template <uint8_t E>
void FloatMetric<E>::saveState()
{
  prefs.putBytes(localId, state, sizeof(state));
}

template <uint8_t E>
void FloatMetric<E>::setValue(float newValue, uint8_t elementIndex)
{
  if (newValue == state[elementIndex] && valid) return;

  valid = true;
  state[elementIndex] = newValue;

  markAsUpdated();
}

template <uint8_t E>
void FloatMetric<E>::setValue(const char *newValue, uint8_t elementIndex)
{
  setValue(strtof(newValue, nullptr), elementIndex);
}

template <uint8_t E>
void FloatMetric<E>::getValue(char *buffer, uint8_t elementIndex)
{
  sprintf(buffer, "%f", getValue(elementIndex));
}

template <uint8_t E>
float FloatMetric<E>::getValue(uint8_t elementIndex)
{
  return state[elementIndex];
}

template <uint8_t E>
void FloatMetric<E>::getValue(JsonArray &json, uint8_t elementIndex)
{
  json.add(getValue(elementIndex));
}

template <uint8_t E>
void FloatMetric<E>::getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex) {
  Metric::getDescriptorData(buffer, bufferIndex, valueDataIndex);

  buffer[bufferIndex++] = static_cast<uint8_t>(precision);
}

template <uint8_t E>
void FloatMetric<E>::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  // TODO: Implement
  // int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
  // buffer[bufferIndex++] = valid;
  // buffer[bufferIndex++] = convertedValue >> 24;
  // buffer[bufferIndex++] = convertedValue >> 16;
  // buffer[bufferIndex++] = convertedValue >> 8;
  // buffer[bufferIndex++] = convertedValue;
}

template <uint8_t E>
uint8_t FloatMetric<E>::getValueDataLength() {
  return 5;
}

// TODO: Figure out a better way to do this...
template class FloatMetric<1>;