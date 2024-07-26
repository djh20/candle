#pragma once

#include "metric.h"

template <uint8_t E>
class StringMetric: public Metric 
{
  public:
    StringMetric(
      const char *domain, const char *localId, MetricType type, uint8_t size
    );
    
    void setValue(const char *value, uint8_t elementIndex = 0) override;

    void getValue(char *buffer, uint8_t elementIndex = 0) override;
    char *getValue(uint8_t elementIndex = 0);
    
    void getValueData(uint8_t *buffer, uint8_t &bufferIndex) override;
    uint8_t getValueDataLength() override;

    void addToJsonDocument(JsonDocument &doc);

  protected:
    void getValue(JsonArray &json, uint8_t elementIndex = 0) override;
  
    void loadState() override;
    void saveState() override;

  private:
    char *state;
    uint8_t stateSize;
    uint8_t elementSize;
};