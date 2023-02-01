#ifndef _VEHICLE_NISSAN_LEAF_H_
#define _VEHICLE_NISSAN_LEAF_H_

#include "vehicle.h"

#define BUS_EV 0

class VehicleNissanLeaf: public Vehicle 
{
  public:
    VehicleNissanLeaf();
    void processFrame(uint8_t &busId, long unsigned int &frameId, unsigned char *frameData);

    MetricInt* gear;
    MetricInt* powered;
    MetricInt* eco;
    MetricInt* socGids;
    MetricInt* soh;
    MetricFloat* powerOutput;
    MetricFloat* socPercent;
    MetricFloat* rearWheelSpeed;
    MetricFloat* leftWheelSpeed;
    MetricFloat* rightWheelSpeed;
    MetricInt* range;
};

#endif
  