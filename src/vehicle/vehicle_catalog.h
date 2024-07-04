#pragma once

#include <vector>
#include "vehicle_entry.h"
#include "vehicle_nissan_leaf.h"

const std::vector<VehicleEntry> vehicleCatalog = {
  {"nissan_leaf", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
};
