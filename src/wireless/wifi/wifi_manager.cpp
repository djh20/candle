#include "wifi_manager.h"
#include "../../config.h"
#include <WiFi.h>

// extern "C" {
// #include <esp_wifi.h>
// }

#define SCAN_INTERVAL 60000

void WiFiManager::begin()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setHostname(GlobalConfig.hostname->getValue());
}

void WiFiManager::loop()
{
  if (!enabled) return;

  uint32_t now = millis();

  if (!WiFi.isConnected() && now - lastScanMillis >= SCAN_INTERVAL)
  {
    WiFi.scanNetworks(true, true);
    lastScanMillis = now;
  }

  processScanResults(WiFi.scanComplete());
}

void WiFiManager::setEnabled(bool enabled)
{
  if (enabled == this->enabled) return;

  // This is only needed if WIFI_OFF doesn't stop scans.
  // if (enabled == false)
  // {
  //   esp_wifi_scan_stop();
  // }

  WiFi.mode(enabled ? WIFI_STA : WIFI_OFF);

  lastScanMillis = 0;
  this->enabled = enabled;
}

void WiFiManager::processScanResults(int16_t totalNetworksFound)
{
  if (totalNetworksFound <= 0) return;

  log_i("Found %i network(s)", totalNetworksFound);

  for (int16_t i = 0; i < totalNetworksFound; i++) {
    const char *ssid = WiFi.SSID(i).c_str();

    if (connectToKnownNetwork(ssid)) break;
  }

  WiFi.scanDelete();
}

bool WiFiManager::connectToKnownNetwork(const char *ssid)
{
  for (uint8_t i = 0; i < GlobalConfig.wifiNetworks->elementCount; i++)
  {
    if (strcmp(ssid, GlobalConfig.wifiNetworks->getValue(i)) == 0)
    {
      log_i("Connecting to %s", ssid);
      WiFi.begin(ssid, GlobalConfig.wifiPasswords->getValue(i));
      return true;
    }
  }
  return false;
}

WiFiManager GlobalWiFiManager;