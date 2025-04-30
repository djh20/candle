#include <Arduino.h>
#include "config.h"
#include "vehicle/vehicle_manager.h"
#include "wireless/wireless_manager.h"
#include "serial/serial_console.h"
// #include "metric/metric_manager.h"

void setup() 
{
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println(R"(____ ____ _  _ ___  _    ____)");
  Serial.println(R"(|    |__| |\ | |  \ |    |___)");
  Serial.println(R"(|___ |  | | \| |__/ |___ |___)");

  Serial.println("Firmware Version: " FIRMWARE_VERSION);
  Serial.println("Hardware Model: " HARDWARE_MODEL);
  Serial.println("Disclaimer: This is a work in progress.");

  GlobalConfig.begin();
  GlobalVehicleManager.begin();
  GlobalWirelessManager.begin();
}

void loop()
{
  #ifdef SERIAL_CONSOLE_ENABLE
  GlobalSerialConsole.loop();
  #endif
  GlobalVehicleManager.loop();
  GlobalWirelessManager.loop();
}