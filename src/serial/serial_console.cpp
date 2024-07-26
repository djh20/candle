#include "serial_console.h"
#include "../console.h"

void SerialConsole::loop()
{
  while (Serial.available() > 0)
  {
    GlobalConsole.processChar(Serial.read());
  }
}

SerialConsole GlobalSerialConsole;