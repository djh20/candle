#pragma once

#include <vector>
#include "vehicle_entry.h"
#include "vehicle_nissan_leaf.h"

std::vector<VehicleEntry> vehicleCatalog = {
  {0x5CB5, "Nissan Leaf (Gen 1)", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
};
