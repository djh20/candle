#pragma once

#include <Arduino.h>
#include "vehicle.h"

class VehicleEntry {
  public:
    VehicleEntry(const char* id, const std::function<Vehicle*()>& callback)
     : id(id), createVehicle(callback) {}

    const char* id;
    std::function<Vehicle*()> createVehicle;
};