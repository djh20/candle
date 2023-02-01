#include "metric.h"

Metric::Metric(const char* id) 
{
  this->id = id;
}

MetricInt::MetricInt(const char* id, int defaultValue) : Metric(id) 
{

}
