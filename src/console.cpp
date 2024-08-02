#include "console.h"
#include "vehicle/vehicle_manager.h"
#include "metric/metric_manager.h"

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
      log_i("List of metrics:");
      for (uint8_t i = 0; i < GlobalMetricManager.totalMetrics; i++)
      {
        Metric *metric = GlobalMetricManager.metrics[i];
        log_i("%s", metric->id);
      }
      return;
    }

    Metric *metric = GlobalMetricManager.getMetric(arg);
    if (metric)
    {
      log_i("Found metric [%s]", metric->id);
    }
    else
    {
      log_w("Failed to find metric [%s]", arg);
      return;
    }

    nextArg(arg);

    if (strcmp(arg, "set") == 0)
    {
      for (uint8_t i = 0; i < metric->elementCount; i++)
      {
        nextArg(arg);
        metric->setValue(arg, i);
      }

      metric->save();
      metric->getState(arg); // Reuse command buffer
      log_i("Set [%s] to [%s]", metric->id, arg);
    }
    else if (strcmp(arg, "modify") == 0)
    {
      nextArg(arg);
      uint8_t element = strtol(arg, nullptr, 0);
      
      nextArg(arg);
      metric->setValue(arg, element);

      metric->save();
      metric->getState(arg); // Reuse command buffer
      log_i("Set [%s] to [%s]", metric->id, arg);
    }
    else if (strcmp(arg, "invalidate") == 0)
    {
      metric->invalidate();
      metric->save();
      log_i("The value of [%s] is no longer valid", metric->id);
    }
    else if (strlen(arg) == 0)
    {
      metric->getState(arg); // Reuse command buffer
      log_i("Current State: %s (%s)", arg, metric->valid ? "valid" : "invalid");
      log_i("Last Updated: %u", metric->lastUpdateMillis);
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
        log_i("List of tasks:");
        for (uint8_t i = 0; i < vehicle->totalTasks; i++)
        {
          Task *task = vehicle->tasks[i];
          log_i("%s", task->id);
        }
        return;
      }

      Task *task = vehicle->getTask(arg);
      if (task)
      {
        log_i("Running task [%s]", task->id);
        vehicle->runTask(task);
      }
      else
      {
        log_w("Failed to find task [%s]", arg);
      }
    }
    else
    {
      log_w("Failed to run task - no vehicle found");
    }
  }
  else if (strcmp(arg, "monitor") == 0)
  {
    Vehicle *vehicle = GlobalVehicleManager.getVehicle();

    if (vehicle)
    {
      nextArg(arg);
      uint8_t busId = strtoul(arg, nullptr, 0);

      nextArg(arg);
      uint16_t msgId = strtoul(arg, nullptr, 0);
      
      CanBus *bus = vehicle->buses[busId];
      bus->setMonitoredMessageId(msgId);

      log_i("Monitoring [%03X] on bus %u", msgId, busId);
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