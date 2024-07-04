#pragma once

#include "metric.h"

class StringMetric: public Metric 
{
  public:
    StringMetric(
      const char *domain, const char *localId, MetricType type, uint8_t size
    );

    void loadValue() override;
    void saveValue() override;
    
    void setValue(const char *newValue);
    void setValueFromString(const char *str) override;
    void getValueAsString(char *str) override;
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;

    char *value;
    uint8_t valueSize;
};