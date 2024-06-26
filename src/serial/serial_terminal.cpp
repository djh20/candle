#include "serial_terminal.h"
#include "../utils.h"
#include "../vehicle/vehicle_manager.h"
#include "../vehicle/vehicle_nissan_leaf.h"

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
  Vehicle *vehicle = GlobalVehicleManager.getVehicle();
  if (!vehicle) return;

  if (strncmp(cmdId, "REQ", SERIAL_CMD_ID_LEN) == 0) 
  {
    log_i("Running request command...");

    uint8_t busId = cmdArgs[0];
    // log_i("Bus ID: %u", busId);
    
    uint16_t resId = cmdArgs[1];
    // log_i("Response ID: %03X", resId);

    uint16_t reqId = cmdArgs[2];
    // log_i("Request ID: %03X", reqId);

    uint8_t reqDataLen = cmdArgs[3];
    // log_i("Request Length: %03X", reqId);

    uint8_t reqData[reqDataLen];
    for (uint8_t i = 0; i < reqDataLen; i++)
    {
      reqData[i] = cmdArgs[i+4];
    }

    PollTask *task = new PollTask(
      vehicle->buses[busId], reqId, reqData, reqDataLen
    );

    task->configureResponse(resId, 10);
    task->maxAttemptDuration = 500;
    task->onFinish = [task]() {
      log_i("Deleting task");
      delete task;
    };

    vehicle->runTask(task);
  }
  else if (strncmp(cmdId, "MON", SERIAL_CMD_ID_LEN) == 0) 
  {
    CanBus *bus = vehicle->buses[cmdArgs[0]];
    bus->setMonitoredMessageId(cmdArgs[1]);
  }
  else if (strncmp(cmdId, "CAP", SERIAL_CMD_ID_LEN) == 0) 
  {
    vehicle->buses[cmdArgs[0]]->capture();
  }
  else if (strncmp(cmdId, "LOG", SERIAL_CMD_ID_LEN) == 0) 
  {
    CanBus *bus = vehicle->buses[cmdArgs[0]];
    
    for (uint8_t i = 0; i < CAN_CAP_LEN; i++)
    {
      uint8_t *frame = bus->captureBuffer[i];
      uint16_t ms = (frame[0] << 8) | frame[1];
      uint16_t frameId = (frame[2] << 8) | frame[3];
      uint16_t frameDataLen = frame[4] >> 1;
      bool tx = frame[4] & 0x01;
      uint8_t *frameData = frame + 5;
      
      log_i(
        "#%02u (%05u) <%u> [%03X]: %02X %02X %02X %02X %02X %02X %02X %02X (%u)",
        i+1, ms, tx, frameId, frameData[0], frameData[1], frameData[2], frameData[3],
        frameData[4], frameData[5], frameData[6], frameData[7], frameDataLen
      );
    }
  }
  else if (strncmp(cmdId, "ACT", SERIAL_CMD_ID_LEN) == 0) 
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (!vehicle) return;

    vehicle->performAction(cmdArgs[0]);
  }
}

SerialTerminal GlobalSerialTerminal;