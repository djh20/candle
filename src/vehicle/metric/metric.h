#ifndef _METRIC_H_
#define _METRIC_H_

#include <ArduinoJson.h>
#define DEFAULT_COOLDOWN 50

class Metric 
{
  public:
    Metric(const char* id, uint16_t cooldown = DEFAULT_COOLDOWN);

    virtual bool update(uint32_t &now);
    bool canUpdate(uint32_t &now);
    virtual void reset();
    virtual void addToJsonDoc(DynamicJsonDocument &doc);

    const char* id;
    uint16_t cooldown;
    uint32_t lastUpdateMillis = 0;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(const char* id, int32_t defaultValue, uint16_t cooldown = DEFAULT_COOLDOWN);

    void setValue(int32_t newValue);
    bool update(uint32_t &now);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    int32_t value;
    int32_t lastUpdateValue;
    int32_t defaultValue;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(const char* id, float defaultValue, uint16_t cooldown = DEFAULT_COOLDOWN);

    void setValue(float newValue);
    bool update(uint32_t &now);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    float value;
    float lastUpdateValue;
    float defaultValue;
};

#endif
