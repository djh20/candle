#pragma once

#include <Arduino.h>

class WiFiManager
{
  public:
    void begin();
    void loop();

    void setEnabled(bool enabled);

  private:
    void processScanResults(int16_t totalNetworksFound);
    bool connectToKnownNetwork(const char *ssid);

    bool enabled = false;
    uint32_t lastScanMillis = 0;
};

extern WiFiManager GlobalWiFiManager;