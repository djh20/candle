#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"

#define WH_PER_GID 80
#define KM_PER_KWH 6.2
#define NOMINAL_PACK_VOLTAGE 360
#define MAX_SOC_PERCENT 95

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle() {}

void VehicleNissanLeaf::registerAll()
{
  registerBus(mainBus = new CanBus(CAN_CS_PIN, CAN_INT_PIN, MCP_ANY, CAN_500KBPS, MCP_8MHZ));

  registerMetric(gear = new MetricInt(METRIC_GEAR, Unit::None));
  registerMetric(soc = new MetricFloat(METRIC_SOC, Unit::Percent, Precision::Medium));
  registerMetric(soh = new MetricFloat(METRIC_SOH, Unit::Percent, Precision::Medium));
  registerMetric(range = new MetricInt(METRIC_RANGE, Unit::Kilometers));
  registerMetric(speed = new MetricFloat(METRIC_SPEED, Unit::KilometersPerHour, Precision::Medium));
  registerMetric(steeringAngle = new MetricFloat(METRIC_STEERING_ANGLE, Unit::None, Precision::High));

  registerMetric(batteryVoltage = new MetricFloat(METRIC_HV_BATT_VOLTAGE, Unit::Volts, Precision::Low));
  registerMetric(batteryCurrent = new MetricFloat(METRIC_HV_BATT_CURRENT, Unit::Amps, Precision::Low));
  registerMetric(batteryPower = new MetricFloat(METRIC_HV_BATT_POWER, Unit::Kilowatts, Precision::Medium));
  registerMetric(batteryCapacity = new MetricFloat(METRIC_HV_BATT_CAPACITY, Unit::KilowattHours, Precision::Medium));
  registerMetric(batteryTemp = new MetricFloat(METRIC_HV_BATT_TEMPERATURE, Unit::Celsius, Precision::Low));

  registerMetric(ambientTemp = new MetricFloat(METRIC_AMBIENT_TEMPERATURE, Unit::Celsius, Precision::Low));
  registerMetric(fanSpeed = new MetricInt(METRIC_FAN_SPEED, Unit::None));
  registerMetric(chargeStatus = new MetricInt(METRIC_CHARGE_STATUS, Unit::None));
  registerMetric(remainingChargeTime = new MetricInt(METRIC_REMAINING_CHARGE_TIME, Unit::Minutes));
  registerMetric(turnSignal = new MetricInt(METRIC_TURN_SIGNAL, Unit::None));
  registerMetric(headlights = new MetricInt(METRIC_HEADLIGHTS, Unit::None));
  registerMetric(parkBrake = new MetricInt(METRIC_PARK_BRAKE, Unit::None));
  registerMetric(quickCharges = new MetricInt(METRIC_QUICK_CHARGES, Unit::None));
  registerMetric(slowCharges = new MetricInt(METRIC_SLOW_CHARGES, Unit::None));

  registerMetric(tripDistance = new MetricInt(METRIC_TRIP_DISTANCE, Unit::Kilometers));
  registerMetric(tripEfficiency = new MetricInt(METRIC_TRIP_EFFICIENCY, Unit::Kilometers));

  registerTask(bmsTask = new PollTask(mainBus, 200, 500, 0x79B, 0x7BB, 6, bmsQuery));
  registerTask(slowChargesTask = new PollTask(mainBus, 300000, 500, 0x797, 0x79A, 1, slowChargesQuery));
  registerTask(quickChargesTask = new PollTask(mainBus, 300000, 500, 0x797, 0x79A, 1, quickChargesQuery));

  //registerTask(sohTask = new PollTask(mainBus, 60000, 500, 0x79B, 0x7BB, 1, sohQuery));
  //registerTask(vcmChargeTask = new PollTask(mainBus, 1000, 500, 0x797, 0x79A, 1, vcmChargeQuery));
}

void VehicleNissanLeaf::processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData)
{
  if (bus == mainBus)
  {
    if (frameId == 0x002) // Steering
    {
      // Cast to int16_t so value becomes negative when turning left.
      int16_t rawSteeringAngle = frameData[0] | (frameData[1] << 8);

      steeringAngle->setValue(constrain(rawSteeringAngle / 6000.0, -1, 1));
    } 
    else if (frameId == 0x60D) // BCM (Body Control Module)
    {
      awake->setValue(((frameData[1] >> 1) & 0x03) >= 2);
    }
    else if (frameId == 0x421) // Instrument Panel Shifter
    {
      byte rawGear = frameData[0];
      
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
    else if (frameId == 0x5B3) // VCM to Cluster
    {
      uint16_t gids = ((frameData[4] & 0x01) << 8) | frameData[5];
      
      // Gids shows as high value on startup - this is incorrect, so we ignore it.
      if (gids < 500) 
      {
        // Range
        double energyKwh = ((gids*WH_PER_GID)/1000.0)-1.15;
        if (energyKwh < 0) energyKwh = 0;
        range->setValue((int32_t)(energyKwh * KM_PER_KWH));
      }

      //uint32_t rawBatteryTemp = (frameData[0] / 2);
      //batteryTemp->setValue((frameData[0] / 2.0), true);
      //batteryTemp->setValue((frameData[0] * 0.25) - 10, true);
      soh->setValue(frameData[1] >> 1);
    }
    else if (frameId == 0x284) // ABS Module
    {
      float frontRightSpeed = ((frameData[0] << 8) | frameData[1]) / 208.0;
      float frontLeftSpeed = ((frameData[2] << 8) | frameData[3]) / 208.0;

      speed->setValue((frontRightSpeed + frontLeftSpeed) / 2.0);
    }
    else if (frameId == 0x358) // Indicators & Headlights
    {
      turnSignal->setValue((frameData[2] & 0x06) / 2);
      headlights->setValue((frameData[1] >> 7) == 1);
    }
    else if (frameId == 0x54B) // Climate Control 1
    {
      fanSpeed->setValue(frameData[4] >> 3);
    }
    else if (frameId == 0x510) // Climate Control 2
    { 
      if (frameData[7] != 0xff)
      {
        ambientTemp->setValue((frameData[7] / 2.0) - 40);
      }
    }
    else if (frameId == 0x5C0) // Lithium Battery Controller (500ms)
    {
      // Battery Temperature as reported by the LBC. Effectively has only
      // 7-bit precision, as the bottom bit is always 0.
      if ((frameData[0] >> 6) == 1) 
      {
        batteryTemp->setValue((frameData[2] / 2.0) - 40);
      }
    }
    else if (frameId == 0x5C5) // Parking Brake & Odometer
    {
      parkBrake->setValue((frameData[0] & 0x04) == 0x04);
      odometer = (frameData[1] << 16) | (frameData[2] << 8) | frameData[3];
      if (tripInProgress) 
      { 
        tripDistance->setValue(odometer - odometerAtLastCharge);

        int16_t idealRemainingRange = rangeAtLastCharge - tripDistance->value;
        tripEfficiency->setValue(range->value - idealRemainingRange);
      }
    }
  }
}

void VehicleNissanLeaf::processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]) {
  if (task == bmsTask)
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
    if (newSoc >= 0 && newSoc <= 100) {
      if (newSoc >= soc->value+5) endTrip(); // Detect if car has been charged.
      soc->setValue(newSoc);
    }

    uint32_t batteryCapacityAh = ((frames[5][2] << 16) | (frames[5][3] << 8) | (frames[5][4])) / 10000.0;
    // Convert Ah to kWh
    batteryCapacity->setValue((batteryCapacityAh * NOMINAL_PACK_VOLTAGE) / 1000.0);
  }
  else if (task == quickChargesTask)
  {
    quickCharges->setValue((frames[0][4] << 8) | frames[0][5]);
  }
  else if (task == slowChargesTask)
  {
    slowCharges->setValue((frames[0][4] << 8) | frames[0][5]);
  }
}

void VehicleNissanLeaf::updateExtraMetrics()
{
  uint32_t now = millis();
  
  // Remaining Charge Time
  if (now - remainingChargeTime->lastUpdateMillis >= 5000)
  {
    if (chargeStatus->value == 1 && batteryPower->value < 0) 
    {
      double percentUntilFull = MAX_SOC_PERCENT - soc->value;

      double energyRequired = batteryCapacity->value * (percentUntilFull/100.0);
      double chargeTimeHours = (energyRequired / -batteryPower->value) * 1.2;

      remainingChargeTime->setValue(chargeTimeHours * 60);
    }
    else
    {
      remainingChargeTime->setValue(0);
    }
  }
}

void VehicleNissanLeaf::metricUpdated(Metric *metric)
{
  if (metric == gear && gear->value > 0)
  {
    startTrip();
    chargeStatus->setValue(0);
  }
  else if (metric == batteryVoltage || metric == batteryCurrent)
  {
    batteryPower->setValue((batteryVoltage->value * batteryCurrent->value) / 1000.0);
  }
  else if (metric == batteryPower)
  {
    if (gear->value == 0 && batteryPower->value <= -1) {
      chargeStatus->setValue(1);
      endTrip();
    }
    else if (chargeStatus->value == 1 && batteryPower->value >= -0.5)
    {
      chargeStatus->setValue(2);
    }
  }
}

void VehicleNissanLeaf::startTrip()
{
  if (tripInProgress) return;

  tripInProgress = true;
  tripDistance->setValue(0);
  tripEfficiency->setValue(0);
  rangeAtLastCharge = range->value;
  odometerAtLastCharge = odometer;
}

void VehicleNissanLeaf::endTrip()
{
  if (!tripInProgress) return;

  tripInProgress = false;
  tripDistance->setValue(0);
  tripEfficiency->invalidate();
  rangeAtLastCharge = 0;
  odometerAtLastCharge = 0;
}

void VehicleNissanLeaf::testCycle()
{
  awake->setValue(1);
  gear->setValue(3);
  soc->setValue(85.5);
  range->setValue(104);
  batteryTemp->setValue(43.5);
  batteryCapacity->setValue(27);
  soh->setValue(87.3);
  tripDistance->setValue(55);
  tripEfficiency->setValue(-5);

  parkBrake->setValue(!parkBrake->value);
  headlights->setValue(!headlights->value);

  float speedValue = speed->value;
  speedValue += 5;
  if (speedValue > 100) speedValue = 0;
  speed->setValue(speedValue);

  float powerValue = batteryPower->value;
  powerValue += 5;
  if (powerValue > 80) powerValue = 0;
  batteryPower->setValue(powerValue);
 
  float steeringValue = steeringAngle->value;
  steeringValue += 0.2;
  if (steeringValue > 1) steeringValue = -1;
  steeringAngle->setValue(steeringValue);
}