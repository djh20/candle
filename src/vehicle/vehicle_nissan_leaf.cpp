#include "vehicle_nissan_leaf.h"

#include <Arduino.h>
#include "can/can_bus.h"

VehicleNissanLeaf::VehicleNissanLeaf() : Vehicle() 
{
  registerBus(new CanBus(BUS_EV, D8, D2, MCP_ANY, CAN_500KBPS, MCP_8MHZ));

  //registerMetric(new MetricInt("powered"));
  //registerMetric(new MetricInt("gear"));
}

void VehicleNissanLeaf::processFrame(uint8_t &busId, long unsigned int &frameId, unsigned char *frameData)
{
  if (busId == BUS_EV) 
  {
    //MetricInt* powered = (MetricInt*) metrics["powered"];
    if (frameId == 0x11a) 
    {
      bool powered = ((frameData[1] & 0x40) >> 6) == 1;
      Serial.print("Powered: ");
      Serial.println(powered);
    }
  }
}
