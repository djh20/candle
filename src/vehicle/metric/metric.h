#ifndef _METRIC_H_
#define _METRIC_H_

class Metric 
{
  public:
    Metric(const char* id);

    const char* id;
};

class MetricInt: public Metric 
{
  public:
    MetricInt(const char* id, int defaultValue = 0);

    int defaultValue;
};

class MetricFloat: public Metric {};

#endif
