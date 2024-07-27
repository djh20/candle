#pragma once

#include "metric.h"

template <uint8_t E>
class StringMetric: public Metric 
{
  public:
    StringMetric(
      const char *domain, const char *localId, MetricType type, uint8_t size
    ) : Metric(domain, localId, type, MetricDataType::String, Unit::None, E)
    {
      stateSize = size*E;
      elementSize = size;

      state = new char[stateSize]();
    }
    
    void setValue(const char *newValue, uint8_t elementIndex = 0) override
    {
      char *currentValue = getValue(elementIndex);

      if (strncmp(currentValue, newValue, elementSize) == 0 && valid) return;

      strncpy(currentValue, newValue, elementSize-1); // -1 for null terminator
      valid = true;

      markAsUpdated();
    }

    void getValue(char *buffer, uint8_t elementIndex = 0) override
    {
      strcpy(buffer, getValue(elementIndex));
    }

    char *getValue(uint8_t elementIndex = 0)
    {
      return state + (elementIndex * elementSize);
    }
    
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override
    {
      // TODO: Implement
    }

    uint8_t getValueDataLength() override
    {
      // TODO: Implement
      return 0;
    }

    void addToJsonDocument(JsonDocument &doc);

  protected:
    void getValue(JsonArray &json, uint8_t elementIndex = 0) override
    {
      json.add(getValue(elementIndex));
    }
  
    void loadState() override
    {
      prefs.getBytes(localId, state, stateSize);
    }

    void saveState() override
    {
      prefs.putBytes(localId, state, stateSize);
    }

  private:
    char *state;
    uint8_t stateSize;
    uint8_t elementSize;
};