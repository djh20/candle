#pragma once

#include <Arduino.h>

class WiFiManager
{
  public:
    void begin();
    void loop();

  private:
    uint32_t lastScanMillis = 0;
};

extern WiFiManager GlobalWiFiManager;