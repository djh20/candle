#pragma once

#include "metric.h"

class IntMetric: public Metric 
{
  public:
    IntMetric(
      const char *domain, const char *localId, MetricType type, Unit unit = Unit::None
    );

    void loadValue() override;
    void saveValue() override;

    void setValue(int32_t newValue);
    void setValueFromString(const char *str) override;
    void getValueAsString(char *str) override;
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;

    int32_t value = 0;
};