#include <Arduino.h>
#include "config.h"
#include "vehicle/vehicle_manager.h"
#include "bluetooth/bluetooth.h"
#include "serial/serial_terminal.h"

#if ARDUHAL_LOG_LEVEL > ARDUHAL_LOG_LEVEL_NONE
#define SERIAL_ENABLE
#endif

void setup() 
{
  #ifdef SERIAL_ENABLE
  Serial.begin(115200);
  delay(1000);

  // Print splash art & info
  Serial.println();
  Serial.println(
    "____ ____ _  _ ___  _    ____\n"
    "|    |__| |\\ | |  \\ |    |___\n"
    "|___ |  | | \\| |__/ |___ |___"
  );

  Serial.println("Firmware Version: " FIRMWARE_VERSION);
  Serial.println("Hardware Model: " HARDWARE_MODEL);
  Serial.println("Disclaimer: This is a work in progress.");

  Serial.println();
  #endif

  GlobalConfig.begin();
  GlobalVehicleManager.begin();
  GlobalBluetooth.begin();
}

void loop()
{
  #ifdef SERIAL_ENABLE
  GlobalSerialTerminal.loop();
  #endif
  
  GlobalVehicleManager.loop();
  GlobalBluetooth.loop();
}