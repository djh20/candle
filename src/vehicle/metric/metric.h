#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

//class Metric;
//typedef std::function<void(Metric*)> MetricUpdateHandler;
//typedef void(*MetricUpdateHandler)(Metric);

class Metric
{
  public:
    Metric(const char* id);

    void onUpdate(std::function<void()> handler);
    virtual void setValueFromString(String str);
    virtual void reset();
    virtual void addToJsonDoc(DynamicJsonDocument &doc);

    const char* id;
    uint32_t lastUpdateMillis = 0;
    std::function<void()> updateHandler;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(const char* id, int32_t defaultValue, int32_t minValue, int32_t maxValue);

    void setValue(int32_t newValue, bool force = false);
    void setValueFromString(String str);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    int32_t value;
    int32_t defaultValue;
    int32_t minValue;
    int32_t maxValue;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(const char* id, float defaultValue, float minValue, float maxValue);

    void setValue(float newValue, bool force = false);
    void setValueFromString(String str);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    float value;
    float defaultValue;
    float minValue;
    float maxValue;
};