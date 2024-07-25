#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager
{
  public:
    void begin();
    void loop();

    void setEnabled(bool enabled);

  private:
    static void onConnect(WiFiEvent_t event, WiFiEventInfo_t info);
    void processScanResults(int16_t totalNetworksFound);

    int8_t getKnownNetworkId(const char *ssid);
    bool isKnownNetwork(const char *ssid);
    const char *getNetworkPassword(const char *ssid);

    bool enabled = false;
    uint32_t lastScanMillis = 0;
    
    char ssid[33];
    uint8_t bssid[6];
};

extern WiFiManager GlobalWiFiManager;