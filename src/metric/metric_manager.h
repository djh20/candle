#pragma once

#include "metric.h"
// #include <map>

class MetricManager
{
  public:
    void registerMetric(Metric *metric);
    Metric *getMetric(const char *id);

    Metric *metrics[64];
    uint8_t totalMetrics = 0;
    uint8_t maxMetricType = 0;
  
  private:
    uint32_t lastSaveMillis = 0;
};

extern MetricManager GlobalMetricManager;