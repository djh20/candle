#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"

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
  registerMetric(rearWheelSpeed = new MetricFloat("rear_wheel_speed", 0));
  registerMetric(leftWheelSpeed = new MetricFloat("left_wheel_speed", 0));
  registerMetric(rightWheelSpeed = new MetricFloat("right_wheel_speed", 0));
  registerMetric(range = new MetricInt("range", 0));
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

      idle = !powered->value;
    }
    else if (frameId == 0x5bc) // Lithium Battery Controller (500ms)
    {
      uint16_t gids = (frameData[0] << 2) | (frameData[1] >> 6);

      if (gids < 1000) { // Gids shows as 1023 on startup; this is incorrect so we ignore it.
        socGids->setValue(gids);
        double energyKwh = ((gids*WH_PER_GID)/1000.0)-1.15;
        if (energyKwh < 0) energyKwh = 0;

        range->setValue((int32_t)(energyKwh * KM_PER_KWH));
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
    else if (frameId == 0x284) // ABS Module
    {
      rearWheelSpeed->setValue(((frameData[4] << 8) | frameData[5]) / 100);
      leftWheelSpeed->setValue(((frameData[2] << 8) | frameData[3]) / 208);
      rightWheelSpeed->setValue(((frameData[0] << 8) | frameData[1]) / 208);
    }
  }
}
