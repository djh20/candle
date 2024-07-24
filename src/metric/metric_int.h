#pragma once

#include "metric.h"

template <uint8_t E>
class IntMetric: public Metric 
{
  public:
    IntMetric(
      const char *domain, const char *localId, MetricType type, Unit unit = Unit::None
    );

    void setValue(int32_t newValue, uint8_t elementIndex = 0);
    void setValue(const char *newValue, uint8_t elementIndex = 0) override;
    int32_t getValue(uint8_t elementIndex = 0);

    void getStateString(char *str) override;
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;

  protected:
    void loadState() override;
    void saveState() override;

  private:
    int32_t state[E] = {};
};