#pragma once

class VehicleManager 
{
  public:
    VehicleManager();

    void begin();
    void loop();
};

extern VehicleManager GlobalVehicleManager;
