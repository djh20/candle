#pragma once

#include "metric.h"

template <uint8_t E>
class FloatMetric: public Metric 
{
  public:
    FloatMetric(
      const char *domain, const char *localId, MetricType type, Unit unit = Unit::None, 
      Precision precision = Precision::Medium
    );

    void setValue(float newValue, uint8_t elementIndex = 0);
    void setValue(const char *newValue, uint8_t elementIndex = 0) override;
    float getValue(uint8_t elementIndex = 0);

    void getStateString(char *str) override;
    void getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex) override;
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;
    
    Precision precision;

  protected:
    void loadState() override;
    void saveState() override;

  private:
    float state[E] = {};
};