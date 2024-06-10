#include "vehicle.h"
#include <Arduino.h>

#define TEST_CYCLE_INTERVAL 200U

Vehicle::Vehicle() {}

void Vehicle::begin()
{
  registerMetric(awake = new MetricInt(METRIC_AWAKE, Unit::None));
  registerAll();
}

void Vehicle::loop()
{
  processBusData();
  processTasks();

  #ifdef TEST_MODE
  uint32_t now = millis();
  if (now - lastTestCycleMillis >= TEST_CYCLE_INTERVAL)
  {
    testCycle();
    lastTestCycleMillis = now;
  }
  #endif

  updateExtraMetrics();
}

void Vehicle::registerBus(CanBus *bus)
{
  buses[totalBuses] = bus;
  totalBuses++;

  bus->init();

  if (!bus->initialized) {
    log_e("Failed to initialize bus %u", totalBuses-1);
    return;
  }

  log_i("Registered bus %u", totalBuses-1);
}

void Vehicle::registerMetric(Metric *metric) 
{
  metrics[totalMetrics++] = metric;
  
  metric->onUpdate([this, metric]() {
    metricUpdated(metric);
  });

  log_i("Registered metric %04X", metric->id);
}

void Vehicle::registerTask(PollTask *task)
{
  log_i("Registered task %u", totalTasks);
  tasks[totalTasks++] = task;
}

void Vehicle::processBusData()
{
  for (uint8_t busIndex = 0; busIndex < totalBuses; busIndex++) 
  {
    CanBus *bus = buses[busIndex];

    if (bus->readFrame()) 
    {
      if (bus->frameId == monitoredMessageId || monitoredMessageId == 0xFFFF)
      {
        log_i(
          "[%03X] (%u): %02X %02X %02X %02X %02X %02X %02X %02X",
          bus->frameId, bus->frameDataLen, bus->frameData[0], bus->frameData[1],
          bus->frameData[2], bus->frameData[3], bus->frameData[4], bus->frameData[5],
          bus->frameData[6], bus->frameData[7]
        );
      }
      
      processFrame(bus, bus->frameId, bus->frameData);

      if (currentTask && currentTask->resId == bus->frameId && currentTask->bus == bus)
      {
        currentTask->processFrame(bus->frameData, bus->frameDataLen);
      }
    }
  }
}

void Vehicle::processTasks()
{
  uint32_t now = millis();

  if (currentTask)
  {
    if (currentTask->isFinished())
    {
      log_i("Task finished");

      if (currentTask->lastRunWasSuccessful)
      {
        processPollResponse(currentTask->bus, currentTask, currentTask->resBuffer);
      }

      // Move queue forward.
      for (uint8_t i = currentTaskIndex; i < totalTasks; i++)
      {
        tasks[i] = tasks[i+1];
      }

      // Only add task back to queue if it's able to run again in the future.
      if (currentTask->canRunAgain())
      {
        tasks[totalTasks-1] = currentTask;
      }
      else
      {
        log_i("Deleted task");
        totalTasks--;
        delete currentTask;
      }

      currentTask = NULL;
    }

    return;
  }

  for (uint8_t i = 0; i < totalTasks; i++)
  {
    PollTask* task = tasks[i];
    if (task->run())
    {
      currentTask = task;
      currentTaskIndex = i;
      break;
    }
  }
}

void Vehicle::setMonitoredMessageId(uint16_t id)
{
  monitoredMessageId = id;
}

void Vehicle::registerAll() {}
void Vehicle::processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData) {}
void Vehicle::processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames) {}
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
void Vehicle::testCycle() {}