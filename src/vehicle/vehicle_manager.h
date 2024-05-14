#pragma once

#include "vehicle.h"

namespace VehicleManager
{
  void begin();
  void loop();

  Vehicle* getVehicle();
}