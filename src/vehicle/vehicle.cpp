#include "vehicle.h"
#include <Arduino.h>

#define TEST_CYCLE_INTERVAL 200U

void Vehicle::begin()
{
  registerMetric(awake = new MetricInt(METRIC_AWAKE, Unit::None));
}

void Vehicle::loop()
{
  handleBuses();
  handleTasks();

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

  bus->begin();

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

void Vehicle::handleBuses()
{
  for (uint8_t i = 0; i < totalBuses; i++) 
  {
    CanBus *bus = buses[i];
    bus->readIncomingFrame();

    if (bus->receivedFrame)
    {
      processFrame(bus, bus->frame.can_id, bus->frame.data);
    }

    // if (bus->readFrame()) 
    // {
    //   processFrame(bus, bus->frameId, bus->frameData);

    //   if (currentTask && currentTask->resId == bus->frameId && currentTask->bus == bus)
    //   {
    //     currentTask->processFrame(bus->frameData, bus->frameDataLen);
    //   }
    // }
  }
}

void Vehicle::runTask(Task *task)
{
  taskQueue[totalTasksInQueue++] = task;
}

void Vehicle::performAction(uint8_t action) {}

void Vehicle::handleTasks()
{
  // uint32_t now = millis();

  if (!currentTask && totalTasksInQueue > 0)
  {
    currentTask = taskQueue[0];
    currentTask->run();
  }

  if (currentTask)
  {
    currentTask->tick();

    if (!currentTask->isRunning())
    {
      // if (currentTask->lastRunWasSuccessful)
      // {
      //   processPollResponse(currentTask->bus, currentTask, currentTask->resBuffer);
      // }

      // Move queue forward.
      for (uint8_t i = 0; i < totalTasksInQueue; i++)
      {
        taskQueue[i] = taskQueue[i+1];
      }

      totalTasksInQueue--;

      // Only add task back to queue if it's able to run again in the future.
      // if (currentTask->canRunAgain())
      // {
      //   tasks[totalTasks-1] = currentTask;
      // }
      // else
      // {
      //   log_i("Deleted task");
      //   totalTasks--;
      //   delete currentTask;
      // }

      currentTask = NULL;
    }
  }

  // for (uint8_t i = 0; i < totalTasksInQueue; i++)
  // {
  //   Task* task = taskQueue[i];
  //   if (task->run())
  //   {
  //     currentTask = task;
  //     currentTaskIndex = i;
  //     break;
  //   }
  // }
}

void Vehicle::processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) {}
// void Vehicle::processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames) {}
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
void Vehicle::testCycle() {}