#include "serial_terminal.h"
#include "../utils.h"
#include "../vehicle/vehicle_manager.h"
#include "../vehicle/vehicle_nissan_leaf.h"
#include "../metric/metric_manager.h"

void SerialTerminal::loop()
{
  while (Serial.available() > 0)
  {
    char incomingChar = Serial.read();

    if (incomingChar == '\n' || incomingChar == ';')
    {
      runCommand();
      waitingForNextArg = false;
      quoted = false;
      cmdBufferIndex = 0;
      memset(cmdBuffer, 0, sizeof(cmdBuffer));
    }
    else if (incomingChar == '"')
    {
      quoted = !quoted;
    }
    else if (incomingChar == ' ' && !quoted)
    {
      if (cmdBufferIndex > 0) // Ignore spaces before command.
      {
        waitingForNextArg = true;
      }
    }
    else if (incomingChar != '\r')
    {
      if (waitingForNextArg)
      {
        cmdBufferIndex++; // Leave gap between arguments (null terminator).
        waitingForNextArg = false;
      }
      cmdBuffer[cmdBufferIndex++] = incomingChar;
    } 
  }
}

void SerialTerminal::runCommand()
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
      nextArg(arg);
      metric->setValue(arg);
      log_i("Set [%s] to [%s]", metric->id, arg);
    }
    else if (strcmp(arg, "invalidate") == 0)
    {
      metric->invalidate();
      log_i("The value of [%s] is no longer valid", metric->id);
    }
    else if (strlen(arg) == 0)
    {
      metric->getValueAsString(arg); // Reuse command buffer
      log_i("Current Value: %s (%s)", arg, metric->valid ? "valid" : "invalid");
      log_i("Last Updated: %u", metric->lastUpdateMillis);
    }
  }
  else if (strcmp(arg, "restart") == 0)
  {
    ESP.restart();
  }
}

void SerialTerminal::nextArg(char *&currentArg)
{
  currentArg += strlen(currentArg) + 1;
}

SerialTerminal GlobalSerialTerminal;