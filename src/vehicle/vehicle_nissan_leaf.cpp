#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"
#include "../utils/logger.h"

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle() 
{
  registerBus(new CanBus(BUS_EV, D8, D2, MCP_ANY, CAN_500KBPS, MCP_8MHZ));

  registerMetric(gear = new MetricInt("gear", 0));
  registerMetric(powered = new MetricInt("powered", 0));
  registerMetric(eco = new MetricInt("eco", 0));
  registerMetric(socGids = new MetricInt("soc_gids", 0));
  registerMetric(soh = new MetricInt("soh", 0));
  registerMetric(powerOutput = new MetricFloat("power_output", 0));
  registerMetric(socPercent = new MetricFloat("soc_percent", 0));
  registerMetric(pluggedIn = new MetricInt("plugged_in", 0));
  registerMetric(speed = new MetricFloat("speed", 0));
  registerMetric(leftSpeed = new MetricFloat("left_speed", 0));
  registerMetric(rightSpeed = new MetricFloat("right_speed", 0));
  registerMetric(batteryTemp = new MetricFloat("battery_temp", 0));
  registerMetric(motorTemp = new MetricFloat("motor_temp", 0));
  registerMetric(inverterTemp = new MetricFloat("inverter_temp", 0));
  registerMetric(ambientTemp = new MetricFloat("ambient_temp", 0));
  registerMetric(fanSpeed = new MetricInt("fan_speed", 0));
  registerMetric(range = new MetricInt("range", 0));
  registerMetric(chargeStatus = new MetricInt("charge_status", 0));
  registerMetric(remainingChargeTime = new MetricInt("remaining_charge_time", 0));
  registerMetric(rangeAtLastCharge = new MetricInt("range_at_last_charge", 0));
  
  registerGps(
    new Gps(
      D4,
      new MetricFloat("gps_lat", 0),
      new MetricFloat("gps_lng", 0),
      new MetricInt("gps_lock", 0),
      new MetricFloat("gps_distance", 0)
    )
  );
}

void VehicleNissanLeaf::processFrame(uint8_t &busId, long unsigned int &frameId, byte *frameData)
{
  if (busId == BUS_EV) 
  {
    if (frameId == 0x11a) // Shift Controller
    {
      gear->setValue((frameData[0] & 0xF0) >> 4);
      powered->setValue((frameData[1] & 0x40) >> 6);
      eco->setValue((frameData[1] & 0x10) >> 4);
    }
    else if (frameId == 0x5bc) // Lithium Battery Controller (500ms)
    {
      uint16_t gids = (frameData[0] << 2) | (frameData[1] >> 6);

      if (gids < 1000) { // Gids shows as 1023 on startup; this is incorrect so we ignore it.
        socGids->setValue(gids);
      }

      soh->setValue((frameData[4] & 0xFE) >> 1);
    }
    else if (frameId == 0x1db) // Lithium Battery Controller (10ms)
    {
      float voltage = ( (frameData[2] << 2) | (frameData[3] >> 6) ) / 2.0F;

      // sent by the LBC, measured inside the battery box
      // current is 11 bit twos complement big endian starting at bit 0

      int16_t rawCurrent = ((int16_t) frameData[0] << 3) | (frameData[1] & 0xe0) >> 5;
      if (rawCurrent & 0x0400) { // negative so extend the sign bit
        rawCurrent |= 0xf800;
      }

      float current = -rawCurrent / 2.0f;
      float power = (current * voltage)/1000.0F;

      if (power > -100 && power < 100) {
        powerOutput->setValue(power);
      }
    }
    else if (frameId == 0x55b) // Lithium Battery Controller (10ms)
    {
      socPercent->setValue(((frameData[0] << 2) | (frameData[1] >> 6)) / 10.0F);
    }
    else if (frameId == 0x1d4) // Vehicle Control Module (10ms)
    {
      uint8_t val = (frameData[6] & 0xE0);
      pluggedIn->setValue((val == 192 || val == 224) ? 1 : 0);
    }
    else if (frameId == 0x284) // ABS Module
    {
      leftSpeed->setValue(((frameData[2] << 8) | frameData[3]) / 208);
      rightSpeed->setValue(((frameData[0] << 8) | frameData[1]) / 208);

      speed->setValue((leftSpeed->value + rightSpeed->value) / 2);
    }
    else if (frameId == 0x5C0) // Lithium Battery Controller (500ms)
    {
      // Battery Temperature as reported by the LBC. Effectively has only
      // 7-bit precision, as the bottom bit is always 0.
      if ((frameData[0] >> 6) == 1) 
      {
        batteryTemp->setValue(((float)frameData[2] / 2) - 40);
      }
    }
    else if (frameId == 0x55a) // Inverter (100ms)
    {
      motorTemp->setValue((5.0 / 9.0) * (frameData[1] - 32));
      inverterTemp->setValue((5.0 / 9.0) * (frameData[2] - 32));
    }
    else if (frameId == 0x54c) // AC Auto Amp (100ms)
    {
      if (frameData[6] != 0xff)
      {
        ambientTemp->setValue(((float)frameData[6] / 2) - 40);
      }
    }
    else if (frameId == 0x54b) // AC Auto Amp (100ms)
    {
      fanSpeed->setValue((frameData[4] & 0xF8) / 8);
    }
  }
}

void VehicleNissanLeaf::updateExtraMetrics()
{
  uint32_t now = millis();

  // Remaining Charge Time
  if (now - remainingChargeTime->lastUpdateMillis >= 5000)
  {
    if (chargeStatus->value == 1 && powerOutput->value < 0) 
    {
      double batteryCapacity = NEW_BATTERY_CAPACITY * (soh->value/100.0);
      double percentUntilFull = MAX_SOC_PERCENT - socPercent->value;

      double energyRequired = batteryCapacity * (percentUntilFull/100.0);
      double chargeTimeHours = energyRequired / -powerOutput->value;

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
  else if (metric == socGids)
  {
    // Range
    double energyKwh = ((socGids->value*WH_PER_GID)/1000.0)-1.15;
    if (energyKwh < 0) energyKwh = 0;
    range->setValue((int32_t)(energyKwh * KM_PER_KWH));
  }
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
}
