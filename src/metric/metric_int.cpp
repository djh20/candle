#include "metric_int.h"

template <uint8_t E>
IntMetric<E>::IntMetric(const char *domain, const char *localId, MetricType type, Unit unit) 
  : Metric(domain, localId, type, MetricDataType::Int, unit, E) {}

template <uint8_t E>
void IntMetric<E>::loadState()
{
  prefs.getBytes(localId, state, sizeof(state));
}

template <uint8_t E>
void IntMetric<E>::saveState()
{
  prefs.putBytes(localId, state, sizeof(state));
}

template <uint8_t E>
void IntMetric<E>::setValue(int32_t newValue, uint8_t elementIndex)
{
  if (newValue == state[elementIndex] && valid) return;

  valid = true;
  state[elementIndex] = newValue;

  markAsUpdated();
}

template <uint8_t E>
void IntMetric<E>::setValue(const char *newValue, uint8_t elementIndex)
{
  setValue(strtol(newValue, nullptr, 0), elementIndex);
}

template <uint8_t E>
int32_t IntMetric<E>::getValue(uint8_t elementIndex)
{
  return state[elementIndex];
}

template <uint8_t E>
void IntMetric<E>::getStateString(char *str)
{
  for (uint8_t i = 0; i < E; i++)
  {
    if (i > 0)
    {
      *str = ',';
      str++;
    }
    
    str += sprintf(str, "%d", state[i]);
  }
}

template <uint8_t E>
void IntMetric<E>::getValueData(uint8_t *buffer, uint8_t &bufferIndex) {
  // TODO: Implement
  // buffer[bufferIndex++] = valid;
  // buffer[bufferIndex++] = value >> 24;
  // buffer[bufferIndex++] = value >> 16;
  // buffer[bufferIndex++] = value >> 8;
  // buffer[bufferIndex++] = value;
}

template <uint8_t E>
uint8_t IntMetric<E>::getValueDataLength() {
  return 5;
}

// TODO: Figure out a better way to do this...
template class IntMetric<1>;