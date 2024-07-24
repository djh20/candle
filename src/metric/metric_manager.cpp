#include "metric_manager.h"

// #define SAVE_INTERVAL 1000

// void MetricManager::loop()
// {
//   uint32_t now = millis();

//   // Save metrics to NVS at the defined interval to prevent excessive wear.
//   // We can't save on every value change because some metrics contain multiple values,
//   // and updating multiple values simultaneously would cause unnecessary writes.
//   if (now - lastSaveMillis >= SAVE_INTERVAL)
//   {
//     for (uint8_t i = 0; i < totalMetrics; i++)
//     {
//       Metric *metric = metrics[i];
//       if (metric->savePending) metric->save();
//     }

//     lastSaveMillis = now;
//   }
// }

void MetricManager::registerMetric(Metric *metric)
{
  metrics[totalMetrics++] = metric;
  metric->begin();
  log_i("Registered metric: %s", metric->id);
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