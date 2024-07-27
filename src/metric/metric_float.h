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

    void setValue(const char *newValue, uint8_t elementIndex = 0) override
    {
      setValue(strtof(newValue, nullptr), elementIndex);
    }

    void getValue(char *buffer, uint8_t elementIndex = 0) override
    {
      sprintf(buffer, "%f", getValue(elementIndex));
    }

    void setValue(float newValue, uint8_t elementIndex = 0)
    {
      if (newValue == getValue(elementIndex) && valid) return;

      valid = true;
      state[elementIndex] = newValue;

      markAsUpdated();
    }

    float getValue(uint8_t elementIndex = 0)
    {
      return state[elementIndex];
    }
    
    void getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex) override
    {
      Metric::getDescriptorData(buffer, bufferIndex, valueDataIndex);
      buffer[bufferIndex++] = static_cast<uint8_t>(precision);
    }

    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override
    {
      // TODO: Implement
      // int32_t convertedValue = value * (float)pow(10, (uint8_t)precision);
      // buffer[bufferIndex++] = valid;
      // buffer[bufferIndex++] = convertedValue >> 24;
      // buffer[bufferIndex++] = convertedValue >> 16;
      // buffer[bufferIndex++] = convertedValue >> 8;
      // buffer[bufferIndex++] = convertedValue;
    }

    uint8_t getValueDataLength() override
    {
      return 5;
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