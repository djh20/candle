#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"
#include "../utils/logger.h"

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle() 
{ 
  registerBus(mainBus = new CanBus(GPIO_NUM_2, GPIO_NUM_4, MCP_ANY, CAN_500KBPS, MCP_8MHZ));

  registerMetric(gear = new MetricInt("gear", 0, 0, 3));
  registerMetric(powered = new MetricInt("powered", 0, 0, 1));
  registerMetric(socGids = new MetricInt("soc_gids", 0, 0, 1000));
  registerMetric(soh = new MetricInt("soh", 0, 0, 100));
  registerMetric(batteryVoltage = new MetricFloat("battery_voltage", 0, 100, 500));
  registerMetric(batteryCurrent = new MetricFloat("battery_current", 0, -300, 300));
  registerMetric(batteryPower = new MetricFloat("battery_power", 0, -70, 100));
  registerMetric(batteryCapacity = new MetricFloat("battery_capacity", 0, 0, 70));
  registerMetric(socPercent = new MetricFloat("soc_percent", 0, 0, 100));
  registerMetric(speed = new MetricFloat("speed", 0, 0, 200)); 
  registerMetric(batteryTemp = new MetricFloat("battery_temp", 0, -10, 100));
  registerMetric(ambientTemp = new MetricFloat("ambient_temp", 0, -10, 100));
  registerMetric(fanSpeed = new MetricInt("fan_speed", 0, 0, 10));
  registerMetric(range = new MetricInt("range", 0, 0, 1000));
  registerMetric(chargeStatus = new MetricInt("charge_status", 0, 0, 2));
  registerMetric(remainingChargeTime = new MetricInt("remaining_charge_time", 0, 0, 999999));
  registerMetric(rangeAtLastCharge = new MetricInt("range_at_last_charge", 0, 0, 1000));
  registerMetric(turnSignal = new MetricInt("turn_signal", 0, 0, 3));
  registerMetric(quickCharges = new MetricInt("quick_charges", 0, 0, 999999));
  registerMetric(slowCharges = new MetricInt("slow_charges", 0, 0, 999999));
  registerMetric(parkingBrake = new MetricInt("parking_brake", 0, 0, 1));

  registerTask(bmsTask = new PollTask(mainBus, 200, 500, 0x79B, 0x7BB, 6, bmsQuery));
  registerTask(slowChargesTask = new PollTask(mainBus, 300000, 500, 0x797, 0x79A, 1, slowChargesQuery));
  registerTask(quickChargesTask = new PollTask(mainBus, 300000, 500, 0x797, 0x79A, 1, quickChargesQuery));

  //registerTask(sohTask = new PollTask(mainBus, 60000, 500, 0x79B, 0x7BB, 1, sohQuery));
  //registerTask(vcmChargeTask = new PollTask(mainBus, 1000, 500, 0x797, 0x79A, 1, vcmChargeQuery));
  
  /*
  registerGps(
    new Gps(
      GPIO_NUM_14,
      new MetricFloat("gps_lat", 0),
      new MetricFloat("gps_lng", 0),
      new MetricInt("gps_lock", 0),
      new MetricFloat("gps_distance", 0)
    )
  );
  */
}

void VehicleNissanLeaf::processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData)
{
  if (bus == mainBus)
  {
    if (frameId == 0x60D) // BCM (Body Control Module)
    {
      powered->setValue(((frameData[1] >> 1) & 0x03) == 3);
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
      uint32_t rawGids = ((frameData[4] & 0x01) << 8) | frameData[5];
      
      // Gids shows as high value on startup - this is incorrect, so we ignore it.
      if (rawGids < 500) 
      {
        socGids->setValue(rawGids);
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
    else if (frameId == 0x5C5) // Parking Brake
    {
      parkingBrake->setValue((frameData[0] & 0x04) == 0x04);
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
    socPercent->setValue(((frames[4][5] << 16) | (frames[4][6] << 8) | frames[4][7]) / 10000.0);

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
  
  /*
  else if (task == sohTask)
  {
    soh->setValue(((frames[0][6] << 8 ) | frames[0][7] ) / 100.0);
  }
  */
  
}

void VehicleNissanLeaf::updateExtraMetrics()
{
  uint32_t now = millis();
  
  // Remaining Charge Time
  if (now - remainingChargeTime->lastUpdateMillis >= 5000)
  {
    if (chargeStatus->value == 1 && batteryPower->value < 0) 
    {
      double percentUntilFull = MAX_SOC_PERCENT - socPercent->value;

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
  if (metric == powered)
  {
    active = powered->value;
  }
  else if (metric == speed)
  {
    moving = (speed->value > 0);
  }
  else if (metric == gear && gear->value > 0)
  {
    if (rangeAtLastCharge->value == 0)
    {
      rangeAtLastCharge->setValue(range->value);
    }

    chargeStatus->reset();
  }
  else if (metric == socGids)
  {
    // Range
    double energyKwh = ((socGids->value*WH_PER_GID)/1000.0)-1.15;
    if (energyKwh < 0) energyKwh = 0;
    range->setValue((int32_t)(energyKwh * KM_PER_KWH));
  }
  else if (metric == batteryVoltage || metric == batteryCurrent)
  {
    batteryPower->setValue((batteryVoltage->value * batteryCurrent->value) / 1000.0);
  }
  else if (metric == batteryPower)
  {
    if (gear->value == 0 && batteryPower->value <= -1) {
      chargeStatus->setValue(1);
      rangeAtLastCharge->reset();
    }
    else if (chargeStatus->value == 1 && batteryPower->value >= -0.5)
    {
      chargeStatus->setValue(2);
    }
  }
  /*
  else if (metric == pluggedIn)
  {
    if (pluggedIn->value)
    {
      if (gps) gps->resetDistance();
    }
  }
  
  if (metric == pluggedIn || metric == powerOutput)
  {
    // Charge Status
    if (pluggedIn->value)
    {
      if (powerOutput->value <= -1)
      {
        chargeStatus->setValue(1);
      }
      else if (powerOutput->value >= 0 && chargeStatus->value == 1)
      {
        chargeStatus->setValue(2);
      }
    }
    else
    {
      chargeStatus->setValue(0);
    }
  }

  if (metric == pluggedIn || metric == gear)
  {
    if (pluggedIn->value)
    {
      rangeAtLastCharge->reset();
    }
    else if (gear->value > 1 && rangeAtLastCharge->value == 0)
    {
      rangeAtLastCharge->setValue(range->value);
    }
  }
  */
}
