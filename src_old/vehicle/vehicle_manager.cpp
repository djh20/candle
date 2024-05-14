#include "vehicle_manager.h"
#include <BLEService.h>
#include "../ble/bluetooth_manager.h"
#include "../ble/uuid.h"
#include "vehicle_catalog.h"

VehicleManager::VehicleManager() {}

void VehicleManager::begin()
{
  BLEService *vehicleCatalogService = GlobalBluetoothManager.server->createService(generateUUID(BLE_SERVICE_VEHICLE_CATALOG), 128U);

  for (const auto& entry : vehicleCatalog)
  {
    BLECharacteristic *characteristic = vehicleCatalogService->createCharacteristic(
      generateUUID(BLE_CHARACTERISTIC_SUPPORTED_VEHICLE, entry.id),
      BLECharacteristic::PROPERTY_READ
    );
    characteristic->setValue(entry.name);
  }
  
  vehicleCatalogService->start();
}

void VehicleManager::selectVehicle(VehicleEntry& entry)
{
  vehicle = entry.createVehicle();
  vehicle->init(GlobalBluetoothManager.server);
}

void VehicleManager::loop()
{

}

// VehicleManager GlobalVehicleManager;
VehicleManager GlobalVehicleManager __attribute__ ((init_priority (4510)));