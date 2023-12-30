#pragma once

#include "vehicle_entry.h"
#include "vehicle_nissan_leaf.h"

#define VEHICLE_CATALOG_LENGTH 20

static VehicleEntry vehicleCatalog[VEHICLE_CATALOG_LENGTH] = {
  {0x5CB5, "Nissan Leaf", []() -> Vehicle* { return new VehicleNissanLeaf(); }},

  // Placeholders for testing
  {0x54E3, "Honda Accord", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x4ECA, "Ford Fusion", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xBFE1, "Chevrolet Malibu", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xFDA9, "Nissan Altima", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x1EA1, "Hyundai Sonata", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x3237, "Kia Optima", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x08BE, "Mazda6", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x5049, "Volkswagen Passat", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x1418, "Subaru Legacy", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xC900, "Chrysler 300", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x11D2, "Dodge Charger", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xF703, "Buick Regal", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xA8DC, "Audi A4", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xABC0, "BMW 3 Series", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x1831, "Mercedes-Benz C-Class", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x53C3, "Lexus ES", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0x2AE6, "Volvo S60", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xC9CA, "Jaguar XE", []() -> Vehicle* { return new VehicleNissanLeaf(); }},
  {0xF819, "Tesla Model 3", []() -> Vehicle* { return new VehicleNissanLeaf(); }}
};
