#ifndef _METRIC_H_
#define _METRIC_H_

#include <ArduinoJson.h>
#define DEFAULT_COOLDOWN 50

class Metric
{
  //typedef std::function<void(Metric *metric)> MetricUpdateHandler;

  public:
    Metric(const char* id, uint16_t cooldown = DEFAULT_COOLDOWN);

    bool isCooldownActive(uint32_t &now);
    //void onUpdate(MetricUpdateHandler updateHandler);

    virtual bool applyValue(bool ignoreCooldown = false);
    virtual void reset();
    virtual void addToJsonDoc(DynamicJsonDocument &doc);

    const char* id;

  protected:
    uint16_t _cooldown;
    uint32_t _lastApplyMillis = 0;
    //MetricUpdateHandler _updateHandler = NULL;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(const char* id, int32_t defaultValue, uint16_t cooldown = DEFAULT_COOLDOWN);

    void setValue(int32_t newValue, bool applyInstantly = false);
    bool applyValue(bool ignoreCooldown = false);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    int32_t value;
    int32_t pendingValue;
    int32_t defaultValue;
};

class MetricFloat: public Metric 
{
  public:
    MetricFloat(const char* id, float defaultValue, uint16_t cooldown = DEFAULT_COOLDOWN);

    void setValue(float newValue, bool applyInstantly = false);
    bool applyValue(bool ignoreCooldown = false);
    void reset();
    void addToJsonDoc(DynamicJsonDocument &doc);

    float value;
    float pendingValue;
    float defaultValue;
};

#endif
