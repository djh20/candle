#include "serial_terminal.h"
#include "../utils.h"
#include "../vehicle/vehicle_manager.h"

void SerialTerminal::loop()
{
  while (Serial.available() > 0)
  {
    char incomingChar = Serial.read();

    if (incomingChar == '\n' || incomingChar == ';')
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
    log_i("Response ID: %03X", resId);

    uint16_t reqId = cmdArgs[2];
    log_i("Request ID: %03X", reqId);

    uint8_t reqData[8];
    for (uint8_t i = 0; i < sizeof(reqData); i++)
    {
      reqData[i] = cmdArgs[i+3];
    }

    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle)
    {
      PollTask *task = new PollTask(
        vehicle->buses[busId], -1, 100, 
        reqId, resId, 8, reqData, true
      );

      vehicle->registerTask(task);
    }
  }
  else if (strncmp(cmdId, "LOG", SERIAL_CMD_ID_LEN) == 0) 
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle) vehicle->setMonitoredMessageId(cmdArgs[0]);
  }
  else if (strncmp(cmdId, "TST", SERIAL_CMD_ID_LEN) == 0) 
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (!vehicle) return;

    CanBus *bus = vehicle->buses[0];

    if (cmdArgs[0] == 1) // Enable Climate Control
    {
      PollTask *wakeTask = new PollTask(
        bus, -1, 50, 0x68C, 0xFFFF, 1,
        wakeRequest, true
      );

      PollTask *climateInitTask = new PollTask(
        bus, -1, 20, 0x56E, 0xFFFF, 1,
        climateOnRequest, true
      );

      PollTask *climateCompeteTask = new PollTask(
        bus, 0, 60, 0x56E, 0xFFFF, 1,
        climateOnRequest, true
      );

      climateCompeteTask->setRunLimit(45);

      vehicle->registerTask(wakeTask);
      vehicle->registerTask(climateInitTask);
      vehicle->registerTask(climateCompeteTask);
    }
    else if (cmdArgs[0] == 2) // Disable Climate Control
    {
      PollTask *climateTask = new PollTask(
        bus, -1, 100, 0x56E, 0xFFFF, 1,
        climateOffRequest, true
      );

      vehicle->registerTask(climateTask);
    }
    else if (cmdArgs[0] == 3) // Continuous Wake (Method 1 - Standard wake msg)
    {
      PollTask *wakeTask = new PollTask(
        bus, 0, 100, 0x68C, 0xFFFF, 1,
        wakeRequest, true
      );

      wakeTask->setRunLimit(100);
      vehicle->registerTask(wakeTask);
    }
    else if (cmdArgs[0] == 4) // Continuous Wake (Method 2 - Climate OFF)
    {
      PollTask *climateTask = new PollTask(
        bus, 0, 100, 0x56E, 0xFFFF, 1,
        climateOffRequest, true
      );

      climateTask->setRunLimit(100);
      vehicle->registerTask(climateTask);
    }
  }
}

SerialTerminal GlobalSerialTerminal;