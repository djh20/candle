#pragma once

#include "metric.h"

class FloatMetric: public Metric 
{
  public:
    FloatMetric(
      const char *domain, const char *localId, MetricType type, Unit unit = Unit::None, 
      Precision precision = Precision::Medium
    );

    void setValue(float newValue);
    void setValue(const char *newValue) override;
    void getValueAsString(char *str) override;
    void getDescriptorData(uint8_t *buffer, uint8_t &bufferIndex, uint8_t valueDataIndex) override;
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;
 
    float value = 0;
    Precision precision;

  protected:
    void loadValue() override;
    void saveValue() override;
};