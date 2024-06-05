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
  handleTasks();
  processBusData();

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
  uint8_t id = totalBusses;
  bus->init();

  if (!bus->initialized) {
    log_e("Failed to initialize bus %u", id);
    return;
  }

  busses[totalBusses++] = bus;
  log_i("Registered bus %u", id);
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
  uint8_t id = totalTasks;
  tasks[totalTasks++] = task;
  log_i("Registered task %u", id);
}

void Vehicle::processBusData()
{
  for (uint8_t busIndex = 0; busIndex < totalBusses; busIndex++) 
  {
    CanBus *bus = busses[busIndex];

    // Attempt to read 10 frames to ensure we don't miss any.
    // TODO: Determine if this is necessary.
    for (uint8_t i = 0; i < 10; i++)
    {
      bool gotFrame = bus->readFrame();
      if (gotFrame) 
      {
        processFrame(bus, bus->frameId, bus->frameData);
        if (currentTask != NULL && currentTask->responseId == bus->frameId && currentTask->bus == bus)
        {
          bool taskCompleted = currentTask->processFrame(bus->frameData);
          if (taskCompleted)
          {
            log_d("Task %u completed", currentTaskIndex);
            processPollResponse(bus, currentTask, currentTask->buffer);
          }
        }
      }
      else break;
    }
  }
}

void Vehicle::handleTasks()
{
  uint32_t now = millis();

  if (currentTask != NULL)
  {
    if (!currentTask->running) {
      currentTask = NULL;
    }
    else if (now - currentTask->lastRunMillis >= currentTask->timeout)
    {
      //Logger.log(Debug, "vehicle", "Cancelling task %u", currentTaskIndex);
      currentTask->cancel();
      currentTask = NULL;
    }
    else return;
  }

  // Only run tasks if vehicle awake for at least 2 seconds.
  if (awake->value && (now - awake->lastUpdateMillis) >= 2000) {
    for (uint8_t offset = 0; offset < totalTasks; offset++)
    {
      uint8_t i = (currentTaskIndex + offset) % totalTasks;

      PollTask* task = tasks[i];
      if (now >= task->nextRunMillis) {
        currentTask = task;
        currentTaskIndex = i;
        //Logger.log(Debug, "vehicle", "Running task %u", i);
        task->run();
        return;
      }
    }
  }
}

void Vehicle::registerAll() {}
void Vehicle::processFrame(CanBus *bus, long unsigned int &frameId, uint8_t *frameData) {}
void Vehicle::processPollResponse(CanBus *bus, PollTask *task, uint8_t frames[][8]) {}
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
void Vehicle::testCycle() {}