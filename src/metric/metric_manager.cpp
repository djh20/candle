#include "metric_manager.h"

void MetricManager::registerMetric(Metric *metric)
{
  metrics[totalMetrics++] = metric;
  metric->begin();
  log_i("Registered metric: [%s]", metric->id);
}

Metric *MetricManager::getMetric(const char *id)
{
  for (uint8_t i = 0; i < totalMetrics; i++)
  {
    Metric *metric = metrics[i];
    if (strcmp(id, metric->id) == 0)
    {
      return metric;
    }
  }

  return nullptr;
}

MetricManager GlobalMetricManager;