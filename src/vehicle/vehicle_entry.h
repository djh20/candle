#pragma once

#include <Arduino.h>
#include "vehicle.h"

class VehicleEntry {
  public:
    VehicleEntry(uint16_t id, const char* name, const std::function<Vehicle*()>& callback)
     : id(id), name(name), createVehicle(callback) {}

    uint16_t id;
    const char* name;
    std::function<Vehicle*()> createVehicle;
};