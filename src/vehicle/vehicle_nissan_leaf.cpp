#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"
#include "metric/metric_manager.h"

#define WH_PER_GID 80
#define KM_PER_KWH 6.2f
#define RESERVED_CAPACITY_KWH 0.8f
#define NOMINAL_PACK_VOLTAGE 360
#define PRECON_AUTO_OFF_MS 30*60*1000

#define FID_VCM_REQ 0x797
#define FID_VCM_RES 0x79A

#define FID_LBC_REQ 0x79B
#define FID_LBC_RES 0x7BB

#define FID_TCU 0x56E

enum Model {
  MODEL_ZE0,    // 2011-2012
  MODEL_AZE0_0, // 2013-2014
  MODEL_AZE0_1, // 2015-2016
  MODEL_AZE0_2  // 2016-2017
};

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle("nl") {}

void VehicleNissanLeaf::begin()
{
  Vehicle::begin();

  registerBus(mainBus = new CanBus(CAN_CS_PIN, CAN_INT_PIN, CAN_500KBPS));

  registerMetrics({
    /* Parameters */
    model = new IntMetric<1>(domain, "model", MetricType::Parameter),
    
    /* Driving */
    speed = new FloatMetric<1>(domain, "speed", MetricType::Statistic, Unit::KilometersPerHour),
    gear = new IntMetric<1>(domain, "gear", MetricType::Statistic),
    steeringAngle = new FloatMetric<1>(domain, "steering_angle", MetricType::Statistic, Unit::None, Precision::High),
    odometer = new IntMetric<1>(domain, "odometer", MetricType::Statistic, Unit::Kilometers),
    tripDistance = new IntMetric<1>(domain, "trip_distance", MetricType::Statistic, Unit::Kilometers),
    tripEfficiency = new IntMetric<1>(domain, "trip_efficiency", MetricType::Statistic, Unit::Kilometers),
    cruiseStatus = new IntMetric<1>(domain, "cruise_status", MetricType::Statistic),
    cruiseSpeed = new IntMetric<1>(domain, "cruise_speed", MetricType::Statistic, Unit::KilometersPerHour),

    /* Battery */
    soc = new FloatMetric<1>(domain, "soc", MetricType::Statistic, Unit::Percent),
    soh = new FloatMetric<1>(domain, "soh", MetricType::Statistic, Unit::Percent),
    range = new IntMetric<1>(domain, "range", MetricType::Statistic, Unit::Kilometers),
    batteryVoltage = new FloatMetric<1>(domain, "hvb_voltage", MetricType::Statistic, Unit::Volts, Precision::Low),
    batteryCapacity = new FloatMetric<1>(domain, "hvb_capacity", MetricType::Statistic, Unit::KilowattHours),
    batteryTemp = new FloatMetric<1>(domain, "hvb_temp", MetricType::Statistic, Unit::Celsius, Precision::Low),

    /* Power */
    netPower = new FloatMetric<1>(domain, "net_power", MetricType::Statistic, Unit::Kilowatts),
    motorPower = new FloatMetric<1>(domain, "motor_power", MetricType::Statistic, Unit::Kilowatts),
    ccPower = new FloatMetric<1>(domain, "cc_power", MetricType::Statistic, Unit::Kilowatts),
    auxPower = new FloatMetric<1>(domain, "aux_power", MetricType::Statistic, Unit::Kilowatts),
    chargePower = new FloatMetric<1>(domain, "chg_power", MetricType::Statistic, Unit::Kilowatts),

    /* Climate Control */
    ccStatus = new IntMetric<1>(domain, "cc_status", MetricType::Statistic),
    ccFanSpeed = new IntMetric<1>(domain, "cc_fan_speed", MetricType::Statistic),
    ambientTemp = new FloatMetric<1>(domain, "ambient_temp", MetricType::Statistic, Unit::Celsius, Precision::Low),
    
    /* Vehicle Status */
    turnSignal = new IntMetric<1>(domain, "turn_signal", MetricType::Statistic),
    headlights = new IntMetric<1>(domain, "headlights", MetricType::Statistic),
    parkBrake = new IntMetric<1>(domain, "park_brake", MetricType::Statistic),
    locked = new IntMetric<1>(domain, "locked", MetricType::Statistic),

    /* Charging */
    chargeMode = new IntMetric<1>(domain, "chg_mode", MetricType::Statistic),
    slowChargeCount = new IntMetric<1>(domain, "chg_slow_count", MetricType::Statistic),
    fastChargeCount = new IntMetric<1>(domain, "chg_fast_count", MetricType::Statistic),
  });
  
  static const uint8_t emptyReq[8] = {};
  genericWakeTask = new PollTask("generic_wake", mainBus, 0x682, emptyReq, 1);

  // This task spoofs the BCM wake up signal which causes some ECUs to come out of sleep 
  // mode and begin communicating.
  static const uint8_t gatewayWakeReq[8] = {0x00, 0x03};
  gatewayWakeTask = new PollTask(
    "gateway_wake", mainBus, 0x35D, gatewayWakeReq, sizeof(gatewayWakeReq)
  );

  // This task attempts to keep the bus awake by continuously sending wake requests.
  keepAwakeTask = new MultiTask("keep_awake");
  keepAwakeTask->add(0, genericWakeTask);
  keepAwakeTask->add(0, gatewayWakeTask);
  keepAwakeTask->minAttemptDuration = 50;
  keepAwakeTask->mode = TaskMode::RepeatUntilStopped;
  
  // This task requests battery energy stats from the BMS/LBC.
  static const uint8_t bmsEnergyReq[8] = {0x02, 0x21, 0x01};
  bmsEnergyTask = new PollTask("bms_energy", mainBus, FID_LBC_REQ, bmsEnergyReq, sizeof(bmsEnergyReq));
  bmsEnergyTask->configureResponse(FID_LBC_RES, 6);
  bmsEnergyTask->maxAttemptDuration = 500;
  registerTask(bmsEnergyTask);

  bmsEnergyTaskWakeful = new MultiTask("bms_energy_wakeful");
  bmsEnergyTaskWakeful->add(0, keepAwakeTask, false);
  bmsEnergyTaskWakeful->add(0, bmsEnergyTask);
  registerTask(bmsEnergyTaskWakeful);

  static const uint8_t bmsHealthReq[8] = {0x02, 0x21, 0x61};
  bmsHealthTask = new PollTask("bms_health", mainBus, FID_LBC_REQ, bmsHealthReq, sizeof(bmsHealthReq));
  bmsHealthTask->configureResponse(FID_LBC_RES, 1);
  bmsHealthTask->maxAttemptDuration = 500;
  registerTask(bmsHealthTask);

  static const uint8_t vcmDiagReq[8] = {0x02, 0x10, 0xC0};
  vcmDiagTask = new PollTask(
    "vcm_diag", mainBus, FID_VCM_REQ, vcmDiagReq, sizeof(vcmDiagReq)
  );
  vcmDiagTask->configureResponse(FID_VCM_RES, 1);
  vcmDiagTask->maxAttemptDuration = 500;
  registerTask(vcmDiagTask);

  static const uint8_t slowChargeCountReq[8] = {0x03, 0x22, 0x12, 0x05};
  slowChargeCountTask = new PollTask(
    "slow_charge_count", mainBus, FID_VCM_REQ, slowChargeCountReq, sizeof(slowChargeCountReq)
  );
  slowChargeCountTask->configureResponse(FID_VCM_RES, 1);
  slowChargeCountTask->maxAttemptDuration = 500;
  registerTask(slowChargeCountTask);

  static const uint8_t quickChargeCountReq[8] = {0x03, 0x22, 0x12, 0x03};
  quickChargeCountTask = new PollTask(
    "quick_charge_count", mainBus, FID_VCM_REQ, quickChargeCountReq, sizeof(quickChargeCountReq)
  );
  quickChargeCountTask->configureResponse(FID_VCM_RES, 1);
  quickChargeCountTask->maxAttemptDuration = 500;
  registerTask(quickChargeCountTask);
  
  chargeCountTask = new MultiTask("charge_count");
  chargeCountTask->add(0, vcmDiagTask);
  chargeCountTask->add(1, slowChargeCountTask);
  chargeCountTask->add(2, quickChargeCountTask);
  registerTask(chargeCountTask);

  static const uint8_t chargeModeReq[8] = {0x03, 0x22, 0x11, 0x4E};
  chargeModeTask = new PollTask(
    "charge_mode", mainBus, FID_VCM_REQ, chargeModeReq, sizeof(chargeModeReq)
  );
  chargeModeTask->configureResponse(FID_VCM_RES, 1);
  chargeModeTask->maxAttemptDuration = 500;
  registerTask(chargeModeTask);

  chargeModeTaskWakeful = new MultiTask("charge_mode_wakeful");
  chargeModeTaskWakeful->add(0, genericWakeTask);
  chargeModeTaskWakeful->add(1, vcmDiagTask);
  chargeModeTaskWakeful->add(2, chargeModeTask);
  registerTask(chargeModeTaskWakeful);

  static const uint8_t chargePortReq[8] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x08};
  PollTask *chargePortReqTask = new PollTask(
    "charge_port_req", mainBus, 0x35D, chargePortReq, sizeof(chargePortReq)
  );
  chargePortReqTask->minAttemptDuration = 50;
  chargePortReqTask->minAttempts = 4;

  chargePortTask = new MultiTask("charge_port");
  chargePortTask->add(0, genericWakeTask);
  chargePortTask->add(1, chargePortReqTask);
  registerTask(chargePortTask);

  static const uint8_t ccOnReq[] = {0x4E, 0x08, 0x12, 0x00};
  PollTask *ccOnReqTask = new PollTask("cc_on_req", mainBus, FID_TCU, ccOnReq, sizeof(ccOnReq));
  ccOnReqTask->minAttempts = 10;
  ccOnReqTask->minAttemptDuration = 100;

  ccOnTask = new MultiTask("cc_on");
  ccOnTask->add(0, genericWakeTask);
  ccOnTask->add(1, ccOnReqTask);
  registerTask(ccOnTask);

  static const uint8_t ccOffReq[] = {0x56, 0x00, 0x01, 0x00};
  PollTask *ccOffReqTask = new PollTask("cc_off_req", mainBus, FID_TCU, ccOffReq, sizeof(ccOffReq));
  ccOffReqTask->minAttempts = 10;
  ccOffReqTask->minAttemptDuration = 100;

  ccOffTask = new MultiTask("cc_off");
  ccOffTask->add(0, genericWakeTask);
  ccOffTask->add(1, ccOffReqTask);
  registerTask(ccOffTask);

  static const uint8_t tcuIdleTaskReq[4] = {0x86};
  tcuIdleTask = new PollTask(
    "tcu_idle", mainBus, FID_TCU, tcuIdleTaskReq, sizeof(tcuIdleTaskReq)
  );
  tcuIdleTask->maxAttemptDuration = 0;
  registerTask(tcuIdleTask);

  // Trigger an update event to handle the loaded model parameter.
  metricUpdated(model);

  // Halt cabin preconditioning to prevent potential battery drain.
  // An ESP reboot would cause the system to lose track of the timer,
  // potentially leaving preconditioning active for a very long time.
  runTask(ccOffTask);
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
      locked->setValue((data[2] & 0x08) > 0);
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
    else if (id == 0x260) // Data for instrumentation cluster
    {
      if (model->isValid())
      {
        // Here we use the instrument cluster 'power bubble' data to calculate motor power.
        // The format of this message varies depending on model year (tested on MY2011 and MY2017).
        // I'm assuming that MY2013 and later all have the new format, but this needs further testing.

        bool isNewFormat = model->getValue() >= MODEL_AZE0_0;
        uint16_t offset = isNewFormat ? 2000 : 400; // Value at rest (0 kW)

        // Get raw power and make zero the midpoint (usage is positive, regen is negative).
        int32_t rawPower = ((data[2] << 4) | (data[3] >> 4)) - offset;
        float scalar = 0;

        if (isNewFormat)
        {
          // Scalar of 0.05 for consumption and 0.015 for regen.
          scalar = (rawPower > 0) ? 0.05 : 0.015;
        }
        else
        {
          // Scalar of 0.125 for consumption and regen.
          scalar = 0.125;
        }

        motorPower->setValue(rawPower * scalar);
      }
    }
    else if (id == 0x284) // ABS Module
    {
      float frontRightSpeed = ((data[0] << 8) | data[1]) / 208.0;
      float frontLeftSpeed = ((data[2] << 8) | data[3]) / 208.0;

      speed->setValue((frontRightSpeed + frontLeftSpeed) / 2.0);
    }
    else if (id == 0x358) // Indicators (BCM)
    {
      turnSignal->setValue((data[2] & 0x06) / 2);
    }
    else if (id == 0x625) // Headlights (BCM)
    {
      // 2nd byte:
      // 0x40 = parking lights on
      // 0x60 = headlights on
      // TODO: Add metric state for parking lights.
      
      // headlights->setValue(data[1] != 0x00);
      headlights->setValue((data[1] & 0x20) != 0x00);
    }
    else if (id == 0x54B) // A/C Auto Amp
    {
      ccStatus->setValue((data[1] & 0x40) == 0x40);
      ccFanSpeed->setValue(data[4] >> 3);
    }
    else if (id == 0x510) // A/C Auto Amp
    { 
      if (data[7] != 0xff)
      {
        ambientTemp->setValue((data[7] / 2.0) - 40);
      }

      ccPower->setValue(((data[3] >> 1) & 0x3F) * 0.25);
      auxPower->setValue(((data[4] >> 3) & 0x1F) * 0.1);
      chargeMode->setValue(data[1] & 0x07);
    }
    else if (id == 0x551) // Cruise Control
    {
      uint8_t status = (data[5] >> 4) & 0x07;
      switch (status)
      {
        case 5: // Ready
          status = 1;
          break;

        case 4: // Engaged
          status = 2;
          break;

        default:
          status = 0;
      }
      cruiseStatus->setValue(status);

      uint8_t speed = data[4];
      if (speed < 254)
      {
        speed *= 0.91; // Convert to real speed
        cruiseSpeed->setValue(speed);
      }
      else
      {
        cruiseSpeed->nullify();
      }
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
  if (task == ccOnTask) 
  {
    preconStartMillis = millis();
    preconActive = true;
  }
  else if (task == ccOffTask || task == tcuIdleTask)
  {
    preconActive = false;
  }
}

void VehicleNissanLeaf::onTaskEnd(Task *task)
{
  if ((task == chargeModeTask || task == chargeModeTaskWakeful) && !task->isSuccessful()) {
    chargeMode->setValue(0);
  }
}

void VehicleNissanLeaf::onPollResponse(Task *task, uint8_t **frames)
{
  if (task == bmsEnergyTask)
  {
    float newSoc = ((frames[4][5] << 16) | (frames[4][6] << 8) | frames[4][7]) / 10000.0f;
    if (newSoc >= 0 && newSoc <= 100) soc->setValue(newSoc);

    float batteryCapacityAh = ((frames[5][2] << 16) | (frames[5][3] << 8) | frames[5][4]) / 10000.0f;
    batteryCapacity->setValue(batteryCapacityAh * NOMINAL_PACK_VOLTAGE / 1000.0f); // Convert Ah to kWh

    // Perform simple range calculation based on energy remaining.
    // TODO: Calculate based on driving efficiency.
    float remainingEnergy = (batteryCapacity->getValue() * soc->getValue() / 100.0f) - RESERVED_CAPACITY_KWH;
    remainingEnergy = max(remainingEnergy, 0.0f);
    range->setValue((int32_t)(remainingEnergy * KM_PER_KWH));

    batteryVoltage->setValue(((frames[3][1] << 8) | frames[3][2]) / 100.0);

    if (chargeMode->isValid() && chargeMode->getValue())
    {
      int32_t rawCurrent = (frames[1][3] << 24) | (frames[1][4] << 16 | ((frames[1][5] << 8) | frames[1][6]));
      if (rawCurrent & 0x8000000 == 0x8000000) 
      {
        rawCurrent = rawCurrent | -0x100000000;
      }

      float current = (rawCurrent > 0) ? (rawCurrent / 1024.0) : 0;
      chargePower->setValue((batteryVoltage->getValue() * current) / 1000.0);
    }
  }
  else if (task == bmsHealthTask) 
  {
    if ((frames[0][0] & 0xF0) == 0x10 && frames[0][3] == 0x61) {
      soh->setValue((frames[0][6] << 8 | frames[0][7]) / 100.0f);
    }
  }
  else if (task == chargeModeTask)
  {
    chargeMode->setValue(frames[0][4]);
  }
  else if (task == quickChargeCountTask)
  {
    fastChargeCount->setValue((frames[0][4] << 8) | frames[0][5]);
  }
  else if (task == slowChargeCountTask)
  {
    slowChargeCount->setValue((frames[0][4] << 8) | frames[0][5]);
  }
}

void VehicleNissanLeaf::updateExtraMetrics()
{
  // Temporary solution to automatically halt cabin preconditioning if left on for too long
  if (preconActive && millis() - preconStartMillis > PRECON_AUTO_OFF_MS) {
    runTask(ccOffTask);
    preconActive = false;
  }
  
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
  /* Individual Metrics */
  if (metric == model)
  {
    bool hasChargePortActuator = model->isValid() && model->getValue() >= MODEL_AZE0_0;
    chargePortTask->setEnabled(hasChargePortActuator);

    // Remote climate only works on leafs with the new TCU on CAR-CAN.
    bool supportsPreconditioning = model->isValid() && model->getValue() >= MODEL_AZE0_2;
    ccOnTask->setEnabled(supportsPreconditioning);
    ccOffTask->setEnabled(supportsPreconditioning);
    tcuIdleTask->setEnabled(supportsPreconditioning);

    // Newer leafs wake up the CAR-CAN when charging starts and finishes.
    // This allows for passive charge status detection (no polling necessary).
    // I assume this is only for AZE0-2...
    bool mustPollForChargeMode = model->isValid() && model->getValue() < MODEL_AZE0_2;
    chargeModeTaskWakeful->setEnabled(mustPollForChargeMode);
  }
  else if (metric == ignition)
  {
    bool carOn = ignition->isValid() && ignition->getValue();

    setTaskInterval(bmsEnergyTaskWakeful, carOn ? 5000 : 2*60*1000);
    bmsEnergyTask->maxAttempts = carOn ? 3 : 10;

    setTaskInterval(bmsHealthTask, carOn ? 5*60*1000 : 0);

    setTaskInterval(tcuIdleTask, carOn ? 1000 : 0);
    setTaskInterval(chargeCountTask, carOn ? 5*60*1000 : 0);
    setTaskInterval(chargeModeTaskWakeful, carOn ? 0 : 5*60*1000);

    genericWakeTask->setEnabled(!carOn);
    gatewayWakeTask->setEnabled(!carOn);
    keepAwakeTask->setEnabled(!carOn);
  }
  else if (metric == gear)
  {
    bool notInPark = gear->getValue() > 0;
    if (notInPark) startTrip();
  }
  else if (metric == chargeMode)
  {
    bool charging = chargeMode->isValid() && chargeMode->getValue();
    if (charging)
    {
      endTrip();
    }
    else
    {
      chargePower->setValue(0.0);
    }
  }

  /* Multiple Metrics */
  if (metric == ignition || metric == chargeMode || metric == ccStatus)
  {
    bool carOn = ignition->isValid() && ignition->getValue();
    bool ccOn = ccStatus->isValid() && ccStatus->getValue();
    bool charging = chargeMode->isValid() && chargeMode->getValue();

    bmsEnergyTaskWakeful->setEnabled(carOn || ccOn || charging);
  }
  else if (metric == motorPower || metric == ccPower || metric == auxPower)
  {
    netPower->setValue(motorPower->getValue() + ccPower->getValue() + auxPower->getValue());
  }
}

void VehicleNissanLeaf::runHomeTasks()
{
  if (soc->getValue() < 70 && tripDistance->getValue() >= 5 && !locked->getValue())
  {
    log_i("Automatically opening charge port");
    runTask(chargePortTask);
  }

  endTrip();
}

void VehicleNissanLeaf::startTrip()
{
  if (tripInProgress) return;
  if (!range->isValid() || !odometer->isValid()) return;

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
  tripEfficiency->nullify();
  rangeAtLastCharge = 0;
  odometerAtLastCharge = 0;
}

void VehicleNissanLeaf::testCycle()
{
  uint32_t now = millis();

  if (now - ignition->lastUpdateMillis >= 20000)
  {
    ignition->setValue(!ignition->getValue());
  }

  if (ignition->getValue())
  {
    gear->setValue((gear->getValue() + 1) % 4);
    range->setValue((range->getValue() + 1) % 200);

    soh->setValue(64);
    chargeMode->setValue(0);
    ccStatus->setValue(0);
    locked->setValue(false);
    batteryCapacity->setValue(22.45);
    odometer->setValue(123456);

    // batteryTemp->setValue(43.5);
    // batteryCapacity->setValue(27);
    // tripDistance->setValue(55);
    // tripEfficiency->setValue(-5);

    parkBrake->setValue(!parkBrake->getValue());
    headlights->setValue(!headlights->getValue());

    float socValue = soc->getValue() + 1.5;
    if (socValue > 100) socValue = 0;
    soc->setValue(socValue);

    float speedValue = speed->getValue() + 1.5;
    if (speedValue > 100) speedValue = 0;
    speed->setValue(speedValue);

    float powerValue = netPower->getValue() + 1.5;
    if (powerValue > 80) powerValue = 0;
    netPower->setValue(powerValue);
  
    float steeringValue = steeringAngle->getValue() + 0.2;
    if (steeringValue > 1) steeringValue = -1;
    steeringAngle->setValue(steeringValue);
  }
}