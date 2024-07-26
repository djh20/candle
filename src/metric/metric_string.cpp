#include "metric_string.h"

template <uint8_t E>
StringMetric<E>::StringMetric(
  const char *domain, const char *localId, MetricType type, uint8_t size
) : Metric(domain, localId, type, MetricDataType::String, Unit::None, E) 
{
  stateSize = size*E;
  elementSize = size;

  state = new char[stateSize]();
}

template <uint8_t E>
void StringMetric<E>::loadState()
{
  prefs.getBytes(localId, state, stateSize);
}

template <uint8_t E>
void StringMetric<E>::saveState()
{
  prefs.putBytes(localId, state, stateSize);
}

template <uint8_t E>
void StringMetric<E>::setValue(const char *newValue, uint8_t elementIndex)
{
  char *element = state + (elementIndex * elementSize);

  if (strncmp(element, newValue, elementSize) == 0 && valid) return;

  strncpy(element, newValue, elementSize-1); // -1 for null terminator
  valid = true;

  markAsUpdated();
}

template <uint8_t E>
char *StringMetric<E>::getValue(uint8_t elementIndex)
{
  return state + (elementIndex * elementSize);
}

template <uint8_t E>
void StringMetric<E>::getValue(JsonArray &json, uint8_t elementIndex)
{
  json.add(getValue(elementIndex));
}

template <uint8_t E>
void StringMetric<E>::getState(char *str)
{
  for (uint8_t i = 0; i < E; i++)
  {
    if (i > 0)
    {
      *str = ',';
      str++;
    }
    
    char *element = state + (i * elementSize);
    strncpy(str, element, elementSize);
    str += strnlen(element, elementSize);
  }
}

template <uint8_t E>
void StringMetric<E>::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  // TODO: Implement
}

template <uint8_t E>
uint8_t StringMetric<E>::getValueDataLength() {
  // TODO: Implement
  return 0;
}

// TODO: Figure out a better way to do this...
template class StringMetric<1>;
template class StringMetric<4>;