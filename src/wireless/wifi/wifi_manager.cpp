#include "wifi_manager.h"
#include "wifi_web_server.h"
#include "../../config.h"
#include <vehicle/vehicle.h>
#include <vehicle/vehicle_manager.h>

#define SCAN_INTERVAL 300000 // 5 minutes

void WiFiManager::begin()
{
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.setHostname(GlobalConfig.hostname->getValue());

  WiFi.onEvent(onConnect, ARDUINO_EVENT_WIFI_STA_GOT_IP);
}

void WiFiManager::loop()
{
  if (!enabled) return;

  uint32_t now = millis();

  if (!WiFi.isConnected() && now - lastScanMillis >= SCAN_INTERVAL)
  {
    log_i("Scanning for networks");
    WiFi.scanNetworks(true, true);
    lastScanMillis = now;
  }

  processScanResults(WiFi.scanComplete());
}

void WiFiManager::setEnabled(bool enabled)
{
  if (enabled == this->enabled) return;

  log_i("WiFi is now %s", enabled ? "enabled" : "disabled");

  WiFi.mode(enabled ? WIFI_STA : WIFI_OFF);
  enabled ? GlobalWiFiWebServer.begin() : GlobalWiFiWebServer.end();

  lastScanMillis = millis() - SCAN_INTERVAL;
  this->enabled = enabled;
}

void WiFiManager::onConnect(WiFiEvent_t event, WiFiEventInfo_t info)
{
  log_i(
    "Connected to %s (IP address: %s)", 
    WiFi.SSID().c_str(), 
    WiFi.localIP().toString().c_str()
  );
}

void WiFiManager::processScanResults(int16_t totalNetworksFound)
{
  if (totalNetworksFound <= 0) return;

  log_i("Found %i network(s):", totalNetworksFound);

  int16_t bestNetwork = -1;
  int32_t bestNetworkRssi = -100;

  for (int16_t i = 0; i < totalNetworksFound; i++) 
  {
    memset(ssid, 0, sizeof(ssid));
    strcpy(ssid, WiFi.SSID(i).c_str());

    int32_t rssi = WiFi.RSSI(i);

    log_i("%i. %s (%i dBm)", i+1, ssid, rssi);

    if (isKnownNetwork(ssid) && rssi > bestNetworkRssi) {
      bestNetwork = i;
      bestNetworkRssi = rssi;
    }
  }

  if (bestNetwork != -1)
  {
    log_i("Best network: %i", bestNetwork+1);

    if (bestNetworkRssi > -80)
    {
      Vehicle *vehicle = GlobalVehicleManager.getVehicle();
      if (vehicle) vehicle->runHomeTasks();
    }

    memset(ssid, 0, sizeof(ssid));
    strcpy(ssid, WiFi.SSID(bestNetwork).c_str());
    memcpy(bssid, WiFi.BSSID(bestNetwork), sizeof(bssid));

    const char *password = getNetworkPassword(ssid);

    if (strlen(password) > 0)
    {
      log_i("Connecting to %s...", ssid);
      WiFi.begin(ssid, getNetworkPassword(ssid), 0, bssid);
    }
  }

  WiFi.scanDelete();
}

int8_t WiFiManager::getKnownNetworkId(const char *ssid)
{
  if (strlen(ssid) > 0) 
  {
    for (uint8_t i = 0; i < GlobalConfig.wifiNetworks->elementCount; i++)
    {
      if (strcmp(ssid, GlobalConfig.wifiNetworks->getValue(i)) == 0)
      {
        return i;
      }
    }
  }
  return -1;
}

bool WiFiManager::isKnownNetwork(const char *ssid)
{
  return getKnownNetworkId(ssid) != -1;
}

const char *WiFiManager::getNetworkPassword(const char *ssid)
{
  int8_t networkId = getKnownNetworkId(ssid);

  if (networkId != -1)
  {
    return GlobalConfig.wifiPasswords->getValue(networkId);
  }

  return nullptr;
}

WiFiManager GlobalWiFiManager;