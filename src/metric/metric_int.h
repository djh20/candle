#pragma once

#include "metric.h"

template <uint8_t E>
class IntMetric: public Metric 
{
  public:
    IntMetric(const char *domain, const char *localId, MetricType type, Unit unit = Unit::None)
      : Metric(domain, localId, type, MetricDataType::Int, unit, E) {}

    void setValue(const char *newValue, uint8_t elementIndex = 0) override
    {
      setValue(strtol(newValue, nullptr, 0), elementIndex);
    }

    void getValue(char *buffer, uint8_t elementIndex = 0) override
    {
      sprintf(buffer, "%d", getValue(elementIndex));
    }

    void setValue(int32_t newValue, uint8_t elementIndex = 0)
    {
      if (newValue == getValue(elementIndex) && valid) return;

      valid = true;
      state[elementIndex] = newValue;

      markAsUpdated();
    }

    int32_t getValue(uint8_t elementIndex = 0)
    {
      return state[elementIndex];
    }

    void getStateData(uint8_t *buffer, uint8_t &bufferIndex) override 
    {
      Metric::getStateData(buffer, bufferIndex);
      memcpy(buffer+bufferIndex, state, sizeof(state));
      bufferIndex += sizeof(state);
    }

    uint8_t getStateDataSize() override
    {
      return Metric::getStateDataSize() + sizeof(state);
    }

  protected:
    void getValue(JsonArray &json, uint8_t elementIndex = 0) override
    {
      json.add(getValue(elementIndex));
    }

    void loadState() override
    {
      prefs.getBytes(localId, state, sizeof(state));
    }

    void saveState() override
    {
      prefs.putBytes(localId, state, sizeof(state));
    }

  private:
    int32_t state[E] = {};
};