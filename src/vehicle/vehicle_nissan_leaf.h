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
    MetricInt *rangeAtLastCharge;
    MetricInt *turnSignal;
    MetricInt *headlights;
    MetricInt *parkBrake;
    MetricInt *quickCharges;
    MetricInt *slowCharges;

  protected:
    void registerAll();
    void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    void updateExtraMetrics();
    void metricUpdated(Metric *metric);
    void testCycle();

  private:
    uint8_t bmsQuery[8] = {0x02, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    PollTask *bmsTask;

    uint8_t quickChargesQuery[8] = {0x03, 0x22, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00};
    PollTask *quickChargesTask;

    uint8_t slowChargesQuery[8] = {0x03, 0x22, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00};
    PollTask *slowChargesTask;
};
