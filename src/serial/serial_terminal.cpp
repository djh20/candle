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
  Vehicle *vehicle = GlobalVehicleManager.getVehicle();
  if (!vehicle) return;

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

    PollTask *task = new PollTask(
      vehicle->buses[busId], reqId, reqData, sizeof(reqData)
    );

    task->configureResponse(resId, 10);
    task->setTimeout(500);
    task->setRunLimit(1);
    task->setEnabled(true);

    vehicle->registerTask(task);
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
    
    for (uint8_t i = 0; i < CAN_CAPTURE_BUFFER_LEN; i++)
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
  else if (strncmp(cmdId, "TST", SERIAL_CMD_ID_LEN) == 0) 
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (!vehicle) return;

    CanBus *bus = vehicle->buses[0];

    if (cmdArgs[0] == 1) // Wake (from OVMS)
    {
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
    else if (cmdArgs[0] == 2) // Open Charge Port
    {
      PollTask *task = new PollTask(bus, 0x682, emptyData, 1);
      task->setRunLimit(1);
      task->setTimeout(50);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x351, req351, sizeof(req351));
      task->setRunLimit(1);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x35D, req35D, sizeof(req35D));
      task->setRunLimit(1);
      task->setTimeout(50);
      // task->configureResponse(0x216, 1);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x625, req625, sizeof(req625));
      task->setRunLimit(1);
      task->setEnabled(true);
      vehicle->registerTask(task);
    }
    else if (cmdArgs[0] == 3) // Emulating BCM wake up from door opening
    {
      PollTask *task = new PollTask(bus, 0x682, emptyData, 1);
      task->setRunLimit(1);
      task->setTimeout(50);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x35D, req35D, 8);
      task->setRunLimit(1);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x60D, bcmVehicleStateData, 8);
      task->setRunLimit(1);
      task->setTimeout(50);
      task->setEnabled(true);
      vehicle->registerTask(task);

      task = new PollTask(bus, 0x79B, bmsReq, sizeof(bmsReq));
      task->configureResponse(0x7BB, 6);
      task->setRunLimit(1);
      task->setTimeout(1000);
      task->setEnabled(true);
      vehicle->registerTask(task);
    }
  }
}

SerialTerminal GlobalSerialTerminal;