#pragma once

#include "vehicle.h"
#include "can/poll_task.h"
#include "can/multi_task.h"
#include "../metric/metric_float.h"

class VehicleNissanLeaf: public Vehicle 
{
  public:
    VehicleNissanLeaf();
    
    void begin() override;

    virtual void runHomeTasks();

    CanBus *mainBus;

    IntMetric<1> *modelYear;
    
    IntMetric<1> *gear;
    FloatMetric<1> *soc;
    FloatMetric<1> *soh;
    IntMetric<1> *range;
    FloatMetric<1> *speed;
    FloatMetric<1> *steeringAngle;
    FloatMetric<1> *batteryVoltage;
    FloatMetric<1> *batteryCurrent;
    FloatMetric<1> *batteryPower;
    FloatMetric<1> *batteryCapacity;
    FloatMetric<1> *batteryTemp;
    FloatMetric<1> *ambientTemp;
    IntMetric<1> *ccStatus;
    IntMetric<1> *ccFanSpeed;
    // IntMetric<1> *chargeStatus;
    IntMetric<1> *chargeMode;
    // IntMetric<1> *remainingChargeTime;
    IntMetric<1> *turnSignal;
    IntMetric<1> *headlights;
    IntMetric<1> *parkBrake;
    IntMetric<1> *locked;
    IntMetric<1> *slowCharges;
    IntMetric<1> *fastCharges;
    IntMetric<1> *odometer;
    IntMetric<1> *tripDistance;
    IntMetric<1> *tripEfficiency;

  protected:
    void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) override;
    void onTaskRun(Task *task) override;
    void onPollResponse(Task *task, uint8_t **frames) override;
    void updateExtraMetrics() override;
    void metricUpdated(Metric *metric) override;
    void testCycle() override;

  private:
    void startTrip();
    void endTrip();

    bool tripInProgress = false;
    bool preheating = false;
    uint32_t odometerAtLastCharge = 0;
    uint16_t rangeAtLastCharge = 0;

    PollTask *genericWakeTask;
    PollTask *gatewayWakeTask;
    MultiTask *keepAwakeTask;

    PollTask *bmsReqTask;
    MultiTask *bmsTask;
    // MultiTask *fullBmsTask;

    MultiTask *chargePortTask;
    // MultiTask *fullChargePortTask;

    MultiTask *ccOnTask;
    // MultiTask *fullActivateCcTask;

    MultiTask *ccOffTask;
    PollTask *ccAutoOffTask;
    // MultiTask *fullDeactivateCcTask;

    PollTask *slowChargesTask;
    PollTask *fastChargesTask;
};
