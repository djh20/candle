#pragma once

#include "vehicle_entry.h"
#include "vehicle_nissan_leaf.h"

#define VEHICLE_CATALOG_LENGTH 1

static VehicleEntry vehicleCatalog[VEHICLE_CATALOG_LENGTH] = {
  {0x5CB5, "Nissan Leaf (Gen 1)", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
};
