#ifndef _METRIC_H_
#define _METRIC_H_

#include <ArduinoJson.h>

//class Metric;
//typedef std::function<void(Metric*)> MetricUpdateHandler;
//typedef void(*MetricUpdateHandler)(Metric);

class Metric
{
  public:
    Metric(const char* id);

    virtual void reset();
    virtual void addToJsonDoc(DynamicJsonDocument &doc);

    const char* id;
    uint32_t lastUpdateMillis;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(const char* id, int32_t defaultValue);

    void setValue(int32_t newValue);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    int32_t value;
    int32_t defaultValue;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(const char* id, float defaultValue);

    void setValue(float newValue);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    float value;
    float defaultValue;
};

#endif
