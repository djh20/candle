#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"
#include "metric/metric_manager.h"

#define WH_PER_GID 80
#define KM_PER_KWH 6.2
#define NOMINAL_PACK_VOLTAGE 360
#define MAX_SOC_PERCENT 95

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle("nl") {}

void VehicleNissanLeaf::begin()
{
  Vehicle::begin();

  registerBus(mainBus = new CanBus(CAN_CS_PIN, CAN_INT_PIN, CAN_500KBPS));

  registerMetrics({
    modelYear = new IntMetric<1>(domain, "model_year", MetricType::Parameter),

    gear = new IntMetric<1>(domain, "gear", MetricType::Statistic),
    soc = new FloatMetric<1>(domain, "soc", MetricType::Statistic, Unit::Percent, Precision::Medium),
    soh = new FloatMetric<1>(domain, "soh", MetricType::Statistic, Unit::Percent, Precision::Medium),
    range = new IntMetric<1>(domain, "range", MetricType::Statistic, Unit::Kilometers),
    speed = new FloatMetric<1>(domain, "speed", MetricType::Statistic, Unit::KilometersPerHour, Precision::Medium),
    steeringAngle = new FloatMetric<1>(domain, "steering_angle", MetricType::Statistic, Unit::None, Precision::High),

    batteryVoltage = new FloatMetric<1>(domain, "hvb_voltage", MetricType::Statistic, Unit::Volts, Precision::Low),
    batteryCurrent = new FloatMetric<1>(domain, "hvb_current", MetricType::Statistic, Unit::Amps, Precision::Low),
    batteryPower = new FloatMetric<1>(domain, "hvb_power", MetricType::Statistic, Unit::Kilowatts, Precision::Medium),
    batteryCapacity = new FloatMetric<1>(domain, "hvb_capacity", MetricType::Statistic, Unit::KilowattHours, Precision::Medium),
    batteryTemp = new FloatMetric<1>(domain, "hvb_temp", MetricType::Statistic, Unit::Celsius, Precision::Low),

    ambientTemp = new FloatMetric<1>(domain, "ambient_temp", MetricType::Statistic, Unit::Celsius, Precision::Low),
    ccStatus = new IntMetric<1>(domain, "cc_status", MetricType::Statistic),
    ccFanSpeed = new IntMetric<1>(domain, "cc_fan_speed", MetricType::Statistic),
    // chargeStatus = new IntMetric<1>(domain, "chg_status", MetricType::Statistic),
    chargeMode = new IntMetric<1>(domain, "chg_mode", MetricType::Statistic),
    // remainingChargeTime = new IntMetric<1>(domain, "chg_time_remain", MetricType::Statistic, Unit::Minutes),
    turnSignal = new IntMetric<1>(domain, "turn_signal", MetricType::Statistic),
    headlights = new IntMetric<1>(domain, "headlights", MetricType::Statistic),
    parkBrake = new IntMetric<1>(domain, "park_brake", MetricType::Statistic),

    slowCharges = new IntMetric<1>(domain, "chg_slow_count", MetricType::Statistic),
    fastCharges = new IntMetric<1>(domain, "chg_fast_count", MetricType::Statistic),

    odometer = new IntMetric<1>(domain, "odometer", MetricType::Statistic, Unit::Kilometers),
    tripDistance = new IntMetric<1>(domain, "trip_distance", MetricType::Statistic, Unit::Kilometers),
    tripEfficiency = new IntMetric<1>(domain, "trip_efficiency", MetricType::Statistic, Unit::Kilometers)
  });
  
  uint8_t emptyReq[8] = {};
  genericWakeTask = new PollTask("generic_wake", mainBus, 0x682, emptyReq, 1);

  // This task spoofs the BCM wake up signal which causes some ECUs to come out of sleep 
  // mode and begin communicating.
  uint8_t gatewayWakeReq[8] = {0x00, 0x03};
  gatewayWakeTask = new PollTask(
    "gateway_wake", mainBus, 0x35D, gatewayWakeReq, sizeof(gatewayWakeReq)
  );

  // This task attempts to keep the bus awake by continuously sending wake requests.
  keepAwakeTask = new MultiTask("keep_awake");
  keepAwakeTask->add(0, genericWakeTask);
  keepAwakeTask->add(0, gatewayWakeTask);
  keepAwakeTask->minAttemptDuration = 50;
  keepAwakeTask->mode = TaskMode::RepeatUntilStopped;
  
  // This task requests battery stats from the BMS/LBC.
  uint8_t bmsReq[8] = {0x02, 0x21, 0x01};
  bmsReqTask = new PollTask("bms_req", mainBus, 0x79B, bmsReq, sizeof(bmsReq));
  bmsReqTask->configureResponse(0x7BB, 6);
  bmsReqTask->maxAttemptDuration = 500;
  bmsReqTask->maxAttempts = 10;
  registerTask(bmsReqTask);

  bmsTask = new MultiTask("bms");
  bmsTask->add(0, keepAwakeTask, false);
  bmsTask->add(0, bmsReqTask);
  bmsTask->setEnabled(false);
  registerTask(bmsTask);

  uint8_t slowChargesReq[8] = {0x03, 0x22, 0x12, 0x05};
  slowChargesTask = new PollTask(
    "slow_charges", mainBus, 0x797, slowChargesReq, sizeof(slowChargesReq)
  );
  slowChargesTask->configureResponse(0x79A, 1);
  slowChargesTask->maxAttemptDuration = 500;
  slowChargesTask->setEnabled(false);
  registerTask(slowChargesTask);
  setTaskInterval(slowChargesTask, 60000); // every minute

  uint8_t fastChargesReq[8] = {0x03, 0x22, 0x12, 0x03};
  fastChargesTask = new PollTask(
    "fast_charges", mainBus, 0x797, fastChargesReq, sizeof(fastChargesReq)
  );
  fastChargesTask->configureResponse(0x79A, 1);
  fastChargesTask->maxAttemptDuration = 500;
  fastChargesTask->setEnabled(false);
  registerTask(fastChargesTask);
  setTaskInterval(fastChargesTask, 60000); // every minute

  uint8_t chargePortReq[8] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x08};
  PollTask *chargePortReqTask = new PollTask(
    "charge_port_req", mainBus, 0x35D, chargePortReq, sizeof(chargePortReq)
  );
  chargePortReqTask->minAttemptDuration = 50;
  chargePortReqTask->minAttempts = 4;

  chargePortTask = new MultiTask("charge_port");
  chargePortTask->add(0, genericWakeTask);
  chargePortTask->add(1, chargePortReqTask);
  registerTask(chargePortTask);

  uint8_t ccOnReq[4] = {0x4E, 0x08, 0x12, 0x00};
  PollTask *ccOnReqTask = new PollTask("cc_on_req", mainBus, 0x56E, ccOnReq, sizeof(ccOnReq));
  ccOnReqTask->minAttempts = 20;
  ccOnReqTask->minAttemptDuration = 100;

  ccOnTask = new MultiTask("cc_on");
  ccOnTask->add(0, genericWakeTask);
  ccOnTask->add(1, ccOnReqTask);
  registerTask(ccOnTask);

  uint8_t ccOffReq[4] = {0x56, 0x00, 0x01, 0x00};
  PollTask *ccOffReqTask = new PollTask("cc_off_req", mainBus, 0x56E, ccOffReq, sizeof(ccOffReq));
  ccOffReqTask->minAttempts = 20;
  ccOffReqTask->minAttemptDuration = 100;

  ccOffTask = new MultiTask("cc_off");
  ccOffTask->add(0, genericWakeTask);
  ccOffTask->add(1, ccOffReqTask);
  registerTask(ccOffTask);

  uint8_t ccAutoOffReq[4] = {0x46, 0x08, 0x32, 0x00};
  ccAutoOffTask = new PollTask(
    "cc_auto_off", mainBus, 0x56E, ccAutoOffReq, sizeof(ccAutoOffReq)
  );
  ccAutoOffTask->maxAttemptDuration = 0;
  ccAutoOffTask->setEnabled(false);
  registerTask(ccAutoOffTask);
  setTaskInterval(ccAutoOffTask, 500);
}

void VehicleNissanLeaf::processFrame(CanBus *bus, const uint32_t &id, uint8_t *data)
{
  if (bus == mainBus)
  {
    if (id == 0x002) // Steering
    {
      // Cast to int16_t so value becomes negative when turning left.
      int16_t rawSteeringAngle = data[0] | (data[1] << 8);

      steeringAngle->setValue(constrain(rawSteeringAngle / 6000.0, -1, 1));
    } 
    else if (id == 0x60D) // BCM (Body Control Module)
    {
      ignition->setValue(((data[1] >> 1) & 0x03) >= 2);
    }
    else if (id == 0x421) // Instrument Panel Shifter
    {
      byte rawGear = data[0];
      
      if (rawGear == 16) // Reverse
      {
        gear->setValue(1);
      }
      else if (rawGear == 24) // Neutral
      {
        gear->setValue(2);
      }
      else if (rawGear == 32 || rawGear == 56) // Drive or B/Eco
      {
        gear->setValue(3);
      } 
      else // Park
      {
        gear->setValue(0);
      }
    }
    else if (id == 0x5B3) // VCM to Cluster
    {
      uint16_t gids = ((data[4] & 0x01) << 8) | data[5];
      
      // Gids shows as high value on startup - this is incorrect, so we ignore it.
      if (gids < 500) 
      {
        // Range
        double energyKwh = ((gids*WH_PER_GID)/1000.0)-1.15;
        if (energyKwh < 0) energyKwh = 0;
        range->setValue((int32_t)(energyKwh * KM_PER_KWH));
      }

      //uint32_t rawBatteryTemp = (data[0] / 2);
      //batteryTemp->setValue((data[0] / 2.0), true);
      //batteryTemp->setValue((data[0] * 0.25) - 10, true);
      soh->setValue(data[1] >> 1);
    }
    else if (id == 0x284) // ABS Module
    {
      float frontRightSpeed = ((data[0] << 8) | data[1]) / 208.0;
      float frontLeftSpeed = ((data[2] << 8) | data[3]) / 208.0;

      speed->setValue((frontRightSpeed + frontLeftSpeed) / 2.0);
    }
    else if (id == 0x358) // Indicators & Headlights
    {
      turnSignal->setValue((data[2] & 0x06) / 2);
      headlights->setValue((data[1] >> 7) == 1);
    }
    // else if (id == 0x50A) // A/C Auto Amp
    // {
    //   ccStatus->setValue(data[4]);
    // }
    else if (id == 0x54B) // A/C Auto Amp
    {
      ccStatus->setValue((data[1] & 0xF7) > 0);
      ccFanSpeed->setValue(data[4] >> 3);
    }
    else if (id == 0x510) // A/C Auto Amp
    { 
      if (data[7] != 0xff)
      {
        ambientTemp->setValue((data[7] / 2.0) - 40);
      }

      chargeMode->setValue(data[1] & 0x07);
    }
    else if (id == 0x5C0) // Lithium Battery Controller (500ms)
    {
      // Battery Temperature as reported by the LBC. Effectively has only
      // 7-bit precision, as the bottom bit is always 0.
      if ((data[0] >> 6) == 1) 
      {
        batteryTemp->setValue((data[2] / 2.0) - 40);
      }
    }
    else if (id == 0x5C5) // Parking Brake & Odometer
    {
      parkBrake->setValue((data[0] & 0x04) == 0x04);
      odometer->setValue((data[1] << 16) | (data[2] << 8) | data[3]);
      if (tripInProgress) 
      { 
        tripDistance->setValue(odometer->getValue() - odometerAtLastCharge);

        int16_t idealRemainingRange = rangeAtLastCharge - tripDistance->getValue();
        tripEfficiency->setValue(range->getValue() - idealRemainingRange);
      }
    }
  }
}

void VehicleNissanLeaf::onTaskRun(Task *task)
{
  // if (task == ccOnTask)
  // {
  //   ccAutoOffTask->setEnabled(true);
  // }
  if (task == ccOffTask)
  {
    ccAutoOffTask->setEnabled(false);
  }
}

void VehicleNissanLeaf::onPollResponse(Task *task, uint8_t **frames)
{
  if (task == bmsReqTask)
  {
    int32_t rawCurrentOne = (frames[0][4] << 24) | (frames[0][5] << 16 | ((frames[0][6] << 8) | frames[0][7]));
    if (rawCurrentOne & 0x8000000 == 0x8000000) {
      rawCurrentOne = rawCurrentOne | -0x100000000;
    }

    int32_t rawCurrentTwo = (frames[1][3] << 24) | (frames[1][4] << 16 | ((frames[1][5] << 8) | frames[1][6]));
    if (rawCurrentTwo & 0x8000000 == 0x8000000) {
      rawCurrentTwo = rawCurrentTwo | -0x100000000;
    }

    batteryCurrent->setValue(-(rawCurrentOne + rawCurrentTwo) / 2.0 / 1024.0);
    batteryVoltage->setValue(((frames[3][1] << 8) | frames[3][2]) / 100.0);

    float newSoc = ((frames[4][5] << 16) | (frames[4][6] << 8) | frames[4][7]) / 10000.0;
    if (newSoc >= 0 && newSoc <= 100) soc->setValue(newSoc);

    uint32_t batteryCapacityAh = ((frames[5][2] << 16) | (frames[5][3] << 8) | (frames[5][4])) / 10000.0;
    // Convert Ah to kWh
    batteryCapacity->setValue((batteryCapacityAh * NOMINAL_PACK_VOLTAGE) / 1000.0);
  }
  else if (task == fastChargesTask)
  {
    fastCharges->setValue((frames[0][4] << 8) | frames[0][5]);
  }
  else if (task == slowChargesTask)
  {
    slowCharges->setValue((frames[0][4] << 8) | frames[0][5]);
  }
}

void VehicleNissanLeaf::updateExtraMetrics()
{
  // uint32_t now = millis();
  
  // Remaining Charge Time
  // if (now - remainingChargeTime->lastUpdateMillis >= 5000)
  // {
  //   if (chargeStatus->getValue() == 1 && batteryPower->getValue() < 0) 
  //   {
  //     double percentUntilFull = MAX_SOC_PERCENT - soc->getValue();

  //     double energyRequired = batteryCapacity->getValue() * (percentUntilFull/100.0);
  //     double chargeTimeHours = (energyRequired / -batteryPower->getValue()) * 1.2;

  //     remainingChargeTime->setValue(chargeTimeHours * 60);
  //   }
  //   else
  //   {
  //     remainingChargeTime->setValue(0);
  //   }
  // }
}

void VehicleNissanLeaf::metricUpdated(Metric *metric)
{
  if (metric == ignition || metric == chargeMode)
  {
    bool carOn = ignition->valid && ignition->getValue();
    bool charging = chargeMode->valid && chargeMode->getValue();

    setTaskInterval(bmsTask, carOn ? 200 : 120000);
    bmsTask->setEnabled(carOn || charging);

    slowChargesTask->setEnabled(carOn);
    fastChargesTask->setEnabled(carOn);
    
    genericWakeTask->setEnabled(!carOn);
    gatewayWakeTask->setEnabled(!carOn);
    keepAwakeTask->setEnabled(!carOn);

    if (charging)
    {
      endTrip();
    }

    if (carOn) 
    {
      ccAutoOffTask->setEnabled(false);
    }
  }
  else if (metric == gear)
  {
    if (gear->getValue() > 0)
    {
      startTrip();
      // chargeStatus->setValue(0);
    }
  }
  else if (metric == batteryVoltage || metric == batteryCurrent)
  {
    batteryPower->setValue((batteryVoltage->getValue() * batteryCurrent->getValue()) / 1000.0);
  }
  // else if (metric == batteryPower)
  // {
  //   if (gear->getValue() == 0 && batteryPower->getValue() <= -1) {
  //     chargeStatus->setValue(1);
  //     endTrip();
  //   }
  //   else if (chargeStatus->getValue() == 1 && batteryPower->getValue() >= -0.5)
  //   {
  //     chargeStatus->setValue(2);
  //   }
  // }
  else if (metric == ccStatus)
  {
    if (!ccStatus->valid || ccStatus->getValue() == 0)
    {
      ccAutoOffTask->setEnabled(false);
    }
  }
}

void VehicleNissanLeaf::runHomeTasks()
{
  if (modelYear->valid && modelYear->getValue() >= 2013)
  {
    if (soc->valid && tripDistance->valid) 
    {
      // TODO: We should probably check if the car was used recently in order to prevent
      // the charge port from opening minutes or hours later due to late wifi detection.

      // TODO: Make these thresholds configurable.
      if (soc->getValue() <= 70 && tripDistance->getValue() >= 5)
      {
        log_i("Automatically opening charge port");
        runTask(chargePortTask);
      }
    }
  }

  endTrip();
}

void VehicleNissanLeaf::startTrip()
{
  if (tripInProgress) return;

  tripInProgress = true;
  tripDistance->setValue(0);
  tripEfficiency->setValue(0);
  rangeAtLastCharge = range->getValue();
  odometerAtLastCharge = odometer->getValue();
}

void VehicleNissanLeaf::endTrip()
{
  tripInProgress = false;
  tripDistance->setValue(0);
  tripEfficiency->invalidate();
  rangeAtLastCharge = 0;
  odometerAtLastCharge = 0;
}

void VehicleNissanLeaf::testCycle()
{
  ignition->setValue(1);
  gear->setValue(3);
  soc->setValue(85.5);
  range->setValue(104);
  batteryTemp->setValue(43.5);
  batteryCapacity->setValue(27);
  soh->setValue(87.3);
  tripDistance->setValue(55);
  tripEfficiency->setValue(-5);

  parkBrake->setValue(!parkBrake->getValue());
  headlights->setValue(!headlights->getValue());

  float speedValue = speed->getValue();
  speedValue += 5;
  if (speedValue > 100) speedValue = 0;
  speed->setValue(speedValue);

  float powerValue = batteryPower->getValue();
  powerValue += 5;
  if (powerValue > 80) powerValue = 0;
  batteryPower->setValue(powerValue);
 
  float steeringValue = steeringAngle->getValue();
  steeringValue += 0.2;
  if (steeringValue > 1) steeringValue = -1;
  steeringAngle->setValue(steeringValue);
}