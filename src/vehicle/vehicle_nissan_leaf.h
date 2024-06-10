#pragma once

#include "vehicle.h"

class VehicleNissanLeaf: public Vehicle 
{
  public:
    VehicleNissanLeaf();

    CanBus *mainBus;
    
    MetricInt *gear;
    MetricFloat *soc;
    MetricFloat *soh;
    MetricInt *range;
    MetricFloat *speed;
    MetricFloat *steeringAngle;
    MetricFloat *batteryVoltage;
    MetricFloat *batteryCurrent;
    MetricFloat *batteryPower;
    MetricFloat *batteryCapacity;
    MetricFloat *batteryTemp;
    MetricFloat *ambientTemp;
    MetricInt *fanSpeed;
    MetricInt *chargeStatus;
    MetricInt *remainingChargeTime;
    MetricInt *turnSignal;
    MetricInt *headlights;
    MetricInt *parkBrake;
    MetricInt *quickCharges;
    MetricInt *slowCharges;
    MetricInt *tripDistance;
    MetricInt *tripEfficiency;

  protected:
    void registerAll();
    void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    void updateExtraMetrics();
    void metricUpdated(Metric *metric);
    void testCycle();

  private:
    void startTrip();
    void endTrip();

    bool tripInProgress = false;
    uint32_t odometer = 0;
    uint32_t odometerAtLastCharge = 0;
    uint16_t rangeAtLastCharge = 0;

    PollTask *bmsTask;
    PollTask *slowChargesTask;
    PollTask *quickChargesTask;
};
