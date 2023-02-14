#ifndef _VEHICLE_NISSAN_LEAF_H_
#define _VEHICLE_NISSAN_LEAF_H_

#include "vehicle.h"

#define BUS_EV 0
#define WH_PER_GID 80
#define KM_PER_KWH 6.2
#define NEW_BATTERY_CAPACITY 24
#define MAX_SOC_PERCENT 95

class VehicleNissanLeaf: public Vehicle 
{
  public:
    VehicleNissanLeaf();
    void processFrame(uint8_t &busId, long unsigned int &frameId, unsigned char *frameData);
    void metricUpdated(Metric *metric);

    MetricInt* gear;
    MetricInt* powered;
    MetricInt* eco;
    MetricInt* socGids;
    MetricInt* soh;
    MetricFloat* powerOutput;
    MetricFloat* socPercent;
    MetricFloat* speed;
    MetricFloat* leftSpeed;
    MetricFloat* rightSpeed;
    MetricInt* range;
};

#endif
  