#include "wifi_manager.h"
#include "../config.h"
#include <WiFi.h>

#define WIFI_SCAN_INTERVAL 30000

void WiFiManager::begin()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_STA);
}

void WiFiManager::loop()
{
  uint32_t now = millis();
  
}

WiFiManager GlobalWiFiManager;