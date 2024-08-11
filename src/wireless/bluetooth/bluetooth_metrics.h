#pragma once

#include "vehicle/vehicle.h"
#include "bluetooth_manager.h"
#include <BLECharacteristic.h>
#include <BLEDescriptor.h>

class BluetoothMetrics
{
  public:
    void begin();
    void loop();

  private:
    uint8_t *groupAssignments;
    uint8_t totalGroups = 0;
    BLECharacteristic *characteristics[8] = {nullptr};
    
    uint8_t attributeBuffer[BLE_OPTIMAL_ATTRIBUTE_SIZE];
    uint8_t attributeBufferSize = 0;
    uint32_t lastTransmissionMillis = 0;
};

extern BluetoothMetrics GlobalBluetoothMetrics;