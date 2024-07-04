#include "vehicle_manager.h"
#include "vehicle_catalog.h"
#include "../config.h"

void VehicleManager::begin()
{
  const char *vehicleId = GlobalConfig.vehicleId->value;

  log_i("Searching for vehicle entry with ID: %s", vehicleId);

  for (const VehicleEntry& entry : vehicleCatalog)
  {
    if (strcmp(vehicleId, entry.id) == 0)
    {
      log_i("Instantiating vehicle: %s", entry.id);
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

Vehicle *VehicleManager::getVehicle()
{
  return vehicle;
}

VehicleManager GlobalVehicleManager;