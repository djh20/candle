#pragma once

#include <Arduino.h>

enum class WirelessProtocol {
  None,
  WiFi,
  Bluetooth
};

class WirelessManager
{
  public:
    void begin();
    void loop();

    void setForcedProtocol(WirelessProtocol protocol);

  private:
    void updateProtocols();
    
    WirelessProtocol currentProtocol = WirelessProtocol::None;
    WirelessProtocol forcedProtocol = WirelessProtocol::None;
    // WirelessProtocol targetProtocol = WirelessProtocol::None;
    uint32_t lastSwitchMillis = 0;
};

extern WirelessManager GlobalWirelessManager;