#pragma once

#include "vehicle.h"
#include "vehicle_entry.h"

class VehicleManager
{
  public:
    VehicleManager();

    void begin();
    void loop();

    void selectVehicle(VehicleEntry &entry);

  private:
    Vehicle* vehicle;
};

extern VehicleManager GlobalVehicleManager;
