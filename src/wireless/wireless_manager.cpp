#include "wireless_manager.h"
#include "wifi/wifi_manager.h"
#include "bluetooth/bluetooth_manager.h"
#include "../vehicle/vehicle_manager.h"

#define PROTOCOL_SWITCH_DELAY 500

void WirelessManager::begin()
{
  GlobalBluetoothManager.begin();
  GlobalWiFiManager.begin();
}

void WirelessManager::loop()
{
  uint32_t now = millis();

  WirelessProtocol targetProtocol;
  Vehicle *vehicle = GlobalVehicleManager.getVehicle();

  if (forcedProtocol != WirelessProtocol::None)
  {
    targetProtocol = forcedProtocol;
  } 
  else if (vehicle && vehicle->ignition->valid && !vehicle->ignition->getValue())
  {
    targetProtocol = WirelessProtocol::WiFi;
  }
  else 
  {
    targetProtocol = WirelessProtocol::Bluetooth;
  }
  
  if (currentProtocol != targetProtocol && now - lastSwitchMillis >= PROTOCOL_SWITCH_DELAY)
  {
    if (currentProtocol != WirelessProtocol::None)
    {
      currentProtocol = WirelessProtocol::None;
    }
    else
    {
      currentProtocol = targetProtocol;
    }

    updateProtocols();
    lastSwitchMillis = now;
  }

  GlobalBluetoothManager.loop();
  GlobalWiFiManager.loop();
}

void WirelessManager::setForcedProtocol(WirelessProtocol protocol)
{
  forcedProtocol = protocol;
}

void WirelessManager::updateProtocols()
{
  GlobalBluetoothManager.setEnabled(currentProtocol == WirelessProtocol::Bluetooth);
  GlobalWiFiManager.setEnabled(currentProtocol == WirelessProtocol::WiFi);
}

WirelessManager GlobalWirelessManager;