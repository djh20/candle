#pragma once

#include "metric.h"

template <uint8_t E>
class FloatMetric: public Metric 
{
  public:
    FloatMetric(
      const char *domain, const char *localId, MetricType type, Unit unit = Unit::None, 
      Precision precision = Precision::Medium
    ) : Metric(domain, localId, type, MetricDataType::Float, unit, E)
    {
      this->precision = precision;
    }

    void setValueFromString(const char *newValue, uint8_t elementIndex = 0) override
    {
      setValue(strtof(newValue, nullptr), elementIndex);
    }

    void getValue(char *buffer, uint8_t elementIndex = 0) override
    {
      sprintf(buffer, "%f", getValue(elementIndex));
    }

    void setValue(float newValue, uint8_t elementIndex = 0)
    {
      if (newValue == getValue(elementIndex) && !isNull) return;

      state[elementIndex] = newValue;
      isNull = false;

      markAsUpdated();
    }

    float getValue(uint8_t elementIndex = 0)
    {
      return state[elementIndex];
    }

    void getStateData(uint8_t *buffer, uint8_t &bufferIndex) override
    {
      Metric::getStateData(buffer, bufferIndex);

      for (uint8_t i = 0; i < elementCount; i++)
      {
        int32_t convertedValue = getValue(i) * (float)pow(10, static_cast<uint8_t>(precision));
        memcpy(buffer+bufferIndex, &convertedValue, sizeof(convertedValue));
        bufferIndex += sizeof(convertedValue);
      }
      
      // memcpy(buffer+bufferIndex, state, sizeof(state));
      // bufferIndex += sizeof(state);
    }

    uint8_t getStateDataSize() override
    {
      return Metric::getStateDataSize() + sizeof(state);
    }
    
    void getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t stateDataIndex) override
    {
      Metric::getDescriptorData(buffer, bufferIndex, stateDataIndex);
      buffer[bufferIndex++] = static_cast<uint8_t>(precision);
    }

    uint8_t getDescriptorDataSize() override
    {
      return Metric::getDescriptorDataSize() + 1;
    }
    
    Precision precision;

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
    float state[E] = {};
};