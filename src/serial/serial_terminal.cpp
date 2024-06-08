#include "serial_terminal.h"
#include "../utils.h"
#include "../vehicle/vehicle_manager.h"

void SerialTerminal::loop()
{
  while (Serial.available() > 0)
  {
    char incomingChar = Serial.read();

    if (incomingChar == '\n')
    {
      runCommand();
      cmdArgIndex = 0;
      cmdCharIndex = 0;
      cmdArgReceived = false;
      memset(cmdId, 0, sizeof(cmdId));
      memset(cmdArgs, 0, sizeof(cmdArgs));
    }
    else if (incomingChar == ' ')
    {
      if (cmdArgReceived)
      {
        cmdArgIndex++;
        cmdArgReceived = false;
      }
    }
    else if (incomingChar != '\r')
    {
      if (cmdCharIndex < SERIAL_CMD_ID_LEN)
      {
        cmdId[cmdCharIndex] = incomingChar;
      }
      else
      {
        cmdArgs[cmdArgIndex] <<= 4;
        cmdArgs[cmdArgIndex] |= Utils::hexCharToInt(incomingChar);
        cmdArgReceived = true;
      }

      cmdCharIndex++;
    } 
  }
}

void SerialTerminal::runCommand()
{
  if (strncmp(cmdId, "REQ", SERIAL_CMD_ID_LEN) == 0) 
  {
    log_i("Running request command...");

    uint8_t busId = cmdArgs[0];
    log_i("Bus ID: %u", busId);
    
    uint16_t resId = cmdArgs[1];
    log_i("Response ID: %02X", resId);

    uint8_t resFrameCount = cmdArgs[2];
    log_i("Frame Count: %u", resFrameCount);

    uint8_t reqLength = cmdArgs[3];
    log_i("Request Data Length: %u", reqLength);

    uint16_t reqId = cmdArgs[4];
    log_i("Request ID: %02X", reqId);

    uint8_t reqData[8];
    for (uint8_t i = 0; i < sizeof(reqData); i++)
    {
      reqData[i] = cmdArgs[i+5];
    }

    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle)
    {
      PollTask *task = new PollTask(
        vehicle->buses[busId], -1, 500, reqId, resId, 
        resFrameCount, reqData, reqLength, true
      );

      vehicle->registerTask(task);
    }
  }
  else if (strncmp(cmdId, "LOG", SERIAL_CMD_ID_LEN) == 0) 
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle) vehicle->setMonitoredMessageId(cmdArgs[0]);
  }
}

SerialTerminal GlobalSerialTerminal;