#pragma once

#include "vehicle.h"

class VehicleManager
{
  public:
    void begin();
    void loop();

    Vehicle* getVehicle();

  private:
    Vehicle* vehicle = NULL;
};

extern VehicleManager GlobalVehicleManager;