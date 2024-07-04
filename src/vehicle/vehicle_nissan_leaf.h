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
    void performAction(uint8_t action) override;

    CanBus *mainBus;

    IntMetric *modelYear;
    
    IntMetric *gear;
    FloatMetric *soc;
    FloatMetric *soh;
    IntMetric *range;
    FloatMetric *speed;
    FloatMetric *steeringAngle;
    FloatMetric *batteryVoltage;
    FloatMetric *batteryCurrent;
    FloatMetric *batteryPower;
    FloatMetric *batteryCapacity;
    FloatMetric *batteryTemp;
    FloatMetric *ambientTemp;
    IntMetric *fanSpeed;
    IntMetric *chargeStatus;
    IntMetric *remainingChargeTime;
    IntMetric *turnSignal;
    IntMetric *headlights;
    IntMetric *parkBrake;
    IntMetric *quickCharges;
    IntMetric *slowCharges;
    IntMetric *tripDistance;
    IntMetric *tripEfficiency;

  protected:
    void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) override;
    // void processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames);
    void updateExtraMetrics() override;
    void metricUpdated(Metric *metric) override;
    void testCycle() override;

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
