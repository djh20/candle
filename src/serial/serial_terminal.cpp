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

    uint8_t reqData[CAN_FRAME_MAX_DATA_LEN];
    for (uint8_t i = 0; i < sizeof(reqData); i++)
    {
      reqData[i] = cmdArgs[i+3];
    }

    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle)
    {
      PollTask *task = new PollTask(
        vehicle->buses[busId], reqId, reqData, sizeof(reqData)
      );

      task->configureResponse(resId, 10);
      task->setTimeout(500);
      task->setRunLimit(1);
      task->setEnabled(true);

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

    PollTask *task = new PollTask(bus, 0x679, emptyData, 1);
    task->setRunLimit(2);
    task->setTimeout(10);
    task->setEnabled(true);
    vehicle->registerTask(task);

    task = new PollTask(bus, 0x5C0, emptyData, 8);
    task->setRunLimit(1);
    task->setInterval(30);
    task->waitUntilNextInterval();
    task->setEnabled(true);
    vehicle->registerTask(task);
  }
}

SerialTerminal GlobalSerialTerminal;