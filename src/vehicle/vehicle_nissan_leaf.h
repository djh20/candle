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

    IntMetric<1> *modelVariant;

    FloatMetric<1> *speed;    
    IntMetric<1> *gear;
    FloatMetric<1> *steeringAngle;
    IntMetric<1> *odometer;
    IntMetric<1> *tripDistance;
    IntMetric<1> *tripEfficiency;
    IntMetric<1> *cruiseStatus;
    IntMetric<1> *cruiseSpeed;

    FloatMetric<1> *soc;
    FloatMetric<1> *soh;
    IntMetric<1> *range;
    FloatMetric<1> *batteryVoltage;
    FloatMetric<1> *batteryCapacity;
    FloatMetric<1> *batteryTemp;

    FloatMetric<1> *netPower;
    FloatMetric<1> *motorPower;
    FloatMetric<1> *ccPower;
    FloatMetric<1> *auxPower;
    FloatMetric<1> *chargePower;

    IntMetric<1> *ccStatus;
    IntMetric<1> *ccFanSpeed;
    FloatMetric<1> *ambientTemp;

    IntMetric<1> *turnSignal;
    IntMetric<1> *headlights;
    IntMetric<1> *parkBrake;
    IntMetric<1> *locked;

    IntMetric<1> *chargeMode;
    IntMetric<1> *slowChargeCount;
    IntMetric<1> *fastChargeCount;

  protected:
    void processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) override;
    void onTaskRun(Task *task) override;
    void onTaskEnd(Task *task) override;
    void onPollResponse(Task *task, uint8_t **frames) override;
    void updateExtraMetrics() override;
    void metricUpdated(Metric *metric) override;
    void testCycle() override;

  private:
    void startTrip();
    void endTrip();

    bool tripInProgress = false;
    uint32_t odometerAtLastCharge = 0;
    uint16_t rangeAtLastCharge = 0;

    PollTask *genericWakeTask;
    PollTask *gatewayWakeTask;
    MultiTask *keepAwakeTask;
    
    PollTask *bmsEnergyTask;
    MultiTask *bmsEnergyTaskWakeful;
    PollTask *bmsHealthTask;

    PollTask *vcmDiagTask;
    
    PollTask *slowChargeCountTask;
    PollTask *quickChargeCountTask;
    MultiTask *chargeCountTask;

    PollTask *chargeModeTask;
    MultiTask *chargeModeTaskWakeful;

    MultiTask *chargePortTask;

    MultiTask *ccOnTask;
    MultiTask *ccOffTask;
    PollTask *tcuIdleTask;

    bool preconActive = false;
    uint32_t preconStartMillis;
};
