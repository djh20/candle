#include "console.h"
#include "vehicle/vehicle_manager.h"
#include "metric/metric_manager.h"
#include "vehicle/can/poll_task.h"

void Console::processChar(const char c)
{
  if (c == '\n' || c == '\0' || c == ';')
  {
    runCommand();
    waitingForNextArg = false;
    quoted = false;
    cmdBufferIndex = 0;
    memset(cmdBuffer, 0, sizeof(cmdBuffer));
  }
  else if (c == '"')
  {
    quoted = !quoted;
  }
  else if (c == ' ' && !quoted)
  {
    if (cmdBufferIndex > 0) // Ignore spaces before command.
    {
      waitingForNextArg = true;
    }
  }
  else if (c != '\r')
  {
    if (waitingForNextArg)
    {
      cmdBufferIndex++; // Leave gap between arguments (null terminator).
      waitingForNextArg = false;
    }
    cmdBuffer[cmdBufferIndex++] = c;
  } 
}

void Console::processString(const char *str)
{
  size_t len = strlen(str)+1;

  for (uint8_t i = 0; i < len; i++)
  {
    processChar(str[i]);
  }
}

void Console::runCommand()
{
  char *arg = cmdBuffer;

  if (strcmp(arg, "metric") == 0)
  {
    nextArg(arg);

    if (strlen(arg) == 0)
    {
      Serial.println("List of metrics:");
      for (uint8_t i = 0; i < GlobalMetricManager.totalMetrics; i++)
      {
        Metric *metric = GlobalMetricManager.metrics[i];
        Serial.println(metric->id);
      }
      return;
    }

    Metric *metric = GlobalMetricManager.getMetric(arg);
    if (metric)
    {
      Serial.printf("Found metric [%s]\r\n", metric->id);
    }
    else
    {
      Serial.printf("Failed to find metric [%s]\r\n", arg);
      return;
    }

    nextArg(arg);

    if (strcmp(arg, "set") == 0)
    {
      for (uint8_t i = 0; i < metric->elementCount; i++)
      {
        nextArg(arg);
        metric->setValueFromString(arg, i);
      }

      metric->save();
      metric->getState(arg); // Reuse command buffer
      Serial.printf("Set [%s] to [%s]\r\n", metric->id, arg);
    }
    else if (strcmp(arg, "modify") == 0)
    {
      nextArg(arg);
      uint8_t element = strtol(arg, nullptr, 0);
      
      nextArg(arg);
      metric->setValueFromString(arg, element);

      metric->save();
      metric->getState(arg); // Reuse command buffer
      Serial.printf("Set [%s] to [%s]\r\n", metric->id, arg);
    }
    else if (strcmp(arg, "nullify") == 0)
    {
      metric->nullify();
      metric->save();
      Serial.printf("The value of [%s] is now null\r\n", metric->id);
    }
    else if (strlen(arg) == 0)
    {
      metric->getState(arg); // Reuse command buffer
      Serial.printf("Current State: %s (%s)\r\n", arg, metric->isValid() ? "valid" : "invalid");
      Serial.printf("Last Updated: %u\r\n", metric->lastUpdateMillis);
    }
  }
  else if (strcmp(arg, "task") == 0)
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();
    if (vehicle)
    {
      nextArg(arg);

      if (strlen(arg) == 0)
      {
        Serial.println("List of tasks:");
        for (uint8_t i = 0; i < vehicle->totalTasks; i++)
        {
          Task *task = vehicle->tasks[i];
          Serial.println(task->id);
        }
        return;
      }

      Task *task = vehicle->getTask(arg);
      if (task)
      {
        Serial.printf("Running task [%s]\r\n", task->id);
        vehicle->runTask(task);
      }
      else
      {
        Serial.printf("Failed to find task [%s]\r\n", arg);
      }
    }
    else
    {
      Serial.println("Failed to run task - no vehicle found");
    }
  }
  else if (strcmp(arg, "can") == 0)
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();

    if (vehicle)
    {
      nextArg(arg);
      uint8_t busId = strtoul(arg, nullptr, 0);
      CanBus *bus = vehicle->buses[busId];

      nextArg(arg);

      if (strcmp(arg, "send") == 0)
      {
        nextArg(arg);

        uint32_t frameId = strtoul(arg, nullptr, 0);
        uint8_t frameData[CAN_MAX_DLEN];

        for (uint8_t i = 0; i < sizeof(frameData); i++)
        {
          nextArg(arg);
          frameData[i] = strtoul(arg, nullptr, 0);
        }

        bus->sendFrame(frameId, frameData, sizeof(frameData));
      }
      else if (strcmp(arg, "monitor") == 0)
      {
        nextArg(arg);
        uint16_t msgId = strtoul(arg, nullptr, 0);
        
        bus->setMonitoredMessageId(msgId);

        Serial.printf("Monitoring [%03X] on bus %u\r\n", msgId, busId);
      }
      else if (strcmp(arg, "discover") == 0)
      {
        nextArg(arg);
        if (strcmp(arg, "stop") == 0)
        {
          bus->stopDiscovery();
        }
        else
        {
          bus->startDiscovery();
        }
      }
    }
    else
    {
      Serial.println("Failed to run CAN command - no vehicle found");
    }
  }
  else if (strcmp(arg, "restart") == 0)
  {
    ESP.restart();
  }
}

void Console::nextArg(char *&currentArg)
{
  currentArg += strlen(currentArg) + 1;
}

Console GlobalConsole;