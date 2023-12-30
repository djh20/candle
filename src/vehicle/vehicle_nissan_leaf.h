#pragma once

#include "vehicle.h"

#define WH_PER_GID 80
#define KM_PER_KWH 6.2
#define NOMINAL_PACK_VOLTAGE 360
#define MAX_SOC_PERCENT 95

class VehicleNissanLeaf: public Vehicle 
{
  public:
    VehicleNissanLeaf();
    void registerAll();
    void processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData);
    void processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]);
    void updateExtraMetrics();
    void metricUpdated(Metric *metric);

    CanBus* mainBus;

    MetricInt* gear;
    MetricInt* powered;
    //MetricInt* eco;
    MetricFloat* soc;
    MetricInt* soh;
    MetricFloat* batteryVoltage;
    MetricFloat* batteryCurrent;
    MetricFloat* batteryPower;
    MetricFloat* batteryCapacity;
    MetricFloat* speed;
    MetricFloat* batteryTemp;
    MetricFloat* ambientTemp;
    MetricInt* fanSpeed;
    MetricInt* range;
    MetricInt* chargeStatus;
    MetricInt* remainingChargeTime;
    MetricInt* rangeAtLastCharge;
    MetricInt* turnSignal;
    MetricInt* quickCharges;
    MetricInt* slowCharges;
    MetricInt* parkingBrake;

    uint8_t bmsQuery[8] = {0x02, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    PollTask* bmsTask;

    uint8_t quickChargesQuery[8] = {0x03, 0x22, 0x12, 0x03, 0x00, 0x00, 0x00, 0x00};
    PollTask* quickChargesTask;

    uint8_t slowChargesQuery[8] = {0x03, 0x22, 0x12, 0x05, 0x00, 0x00, 0x00, 0x00};
    PollTask* slowChargesTask;
    
    //uint8_t sohQuery[8] = {0x02, 0x21, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00};
    //PollTask* sohTask;

    //uint8_t vcmChargeQuery[8] = {0x03, 0x22, 0x11, 0x4E, 0x00, 0x00, 0x00, 0x00};
    //PollTask* vcmChargeTask;
};
