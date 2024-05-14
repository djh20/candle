#include "vehicle_manager.h"
#include "vehicle_catalog.h"
#include "../config.h"

static Vehicle* vehicle;

void VehicleManager::begin()
{
  uint16_t vehicleId = Config::getVehicleId();

  log_i("Searching for vehicle entry with ID: %04X", vehicleId);

  for (const VehicleEntry& entry : vehicleCatalog)
  {
    if (entry.id == vehicleId)
    {
      log_i("Instantiating vehicle: %s", entry.name);
      vehicle = entry.createVehicle();
      vehicle->begin();
      break;
    }
  }
}

void VehicleManager::loop()
{
  if (vehicle)
  {
    vehicle->loop();
  }
}

Vehicle* VehicleManager::getVehicle()
{
  return vehicle;
}