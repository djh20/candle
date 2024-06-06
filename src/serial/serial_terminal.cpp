#include "serial_terminal.h"
#include <Arduino.h>
#include "../utils.h"

void SerialTerminal::begin()
{}

void SerialTerminal::loop()
{
  while (Serial.available() > 0)
  {
    char incomingChar = Serial.read();

    if (incomingChar == '\n')
    {
      runCommand();
      commandPos = 0;
      memset(commandData, 0, sizeof(commandData));
    }
    else if (incomingChar != ' ' && incomingChar != '\r')
    {
      if (commandPos < SERIAL_CMD_ID_LEN)
      {
        commandId[commandPos] = incomingChar;
      }
      else
      {
        commandData[commandPos-SERIAL_CMD_ID_LEN] = Utils::hexCharToInt(incomingChar);
      }

      commandPos++;
    } 
  }
}

void SerialTerminal::runCommand()
{
  if (strncmp(commandId, "REQ", SERIAL_CMD_ID_LEN) == 0) 
  {
    // TODO: Send Request

    // Format:  CMD BUS REQ RES FRAMES DATA...
    // Example: REQ 0  79B 7BB  06     02 21 01

    // With 4-bit values:
    // uint8_t busId = commandData[0];
    // uint16_t requestId = (commandData[1] << 8) | (commandData[2] << 4) | commandData[3];
    // uint16_t responseId = (commandData[4] << 8) | (commandData[5] << 4) | commandData[6];
  }
}

SerialTerminal GlobalSerialTerminal;