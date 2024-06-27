#pragma once

#include "vehicle.h"
#include "can/poll_task.h"
#include "can/multi_task.h"

class VehicleNissanLeaf: public Vehicle 
{
  public:
    void begin();
    void performAction(uint8_t action);

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
    void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data);
    // void processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames);
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

    PollTask *genericWakeTask;
    PollTask *gatewayWakeTask;
    MultiTask *keepAwakeTask;

    PollTask *bmsTask;
    MultiTask *fullBmsTask;

    PollTask *chargePortTask;
    MultiTask *fullChargePortTask;

    PollTask *activateCcTask;
    MultiTask *fullActivateCcTask;

    PollTask *deactivateCcTask;
    MultiTask *fullDeactivateCcTask;

    PollTask *slowChargesTask;
    PollTask *quickChargesTask;
};
