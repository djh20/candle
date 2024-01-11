#include "vehicle.h"
#include <Arduino.h>
#include "../utils/logger.h"

Vehicle::Vehicle() {}

void Vehicle::init(BLEServer *bleServer)
{
  this->bleServer = bleServer;
  bleService = bleServer->createService(BLEUUID(METRICS_SERVICE_UUID), 128U, 0);

  registerMetric(awake = new MetricInt(METRIC_AWAKE, Unit::None));
  registerMetric(tripDistance = new MetricFloat(METRIC_TRIP_DISTANCE, Unit::Kilometers, Precision::High));

  registerAll();

  bleService->start();
}

void Vehicle::registerBus(CanBus *bus)
{
  uint8_t id = totalBusses;
  bus->init();

  if (!bus->initialized) {
    Logger.log(Error, "vehicle", "Failed to register bus %u", id);
    return;
  }

  busses[totalBusses++] = bus;
  Logger.log(Info, "vehicle", "Registered bus %u", id);
}

void Vehicle::registerMetric(Metric *metric) 
{
  metrics[totalMetrics++] = metric;

  bleService->addCharacteristic(metric->bleCharacteristic);
  
  metric->onUpdate([this, metric]() {
    metricUpdated(metric);
  });

  Logger.log(Info, "vehicle", "Registered metric %04X", metric->id);
}

void Vehicle::registerTask(PollTask *task)
{
  uint8_t id = totalTasks;
  tasks[totalTasks++] = task;
  Logger.log(Info, "vehicle", "Registered task %u", id);
}

void Vehicle::update()
{
  handleTasks();
  readAndProcessBusData();
  updateExtraMetrics();
}

void Vehicle::readAndProcessBusData()
{
  for (int i = 0; i < totalBusses; i++) 
  {
    CanBus *bus = busses[i];
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
            Logger.log(Debug, "vehicle", "Task %u completed", currentTaskIndex);
            processPollResponse(bus, currentTask, currentTask->buffer);
          }
        }
      }
      else break;
    }
  }
}

void Vehicle::sendUpdatedMetrics(uint32_t sinceMillis)
{
  for (int i = 0; i < totalMetrics; i++)
  {
    Metric* metric = metrics[i];
    bool updated = (metric->lastUpdateMillis >= sinceMillis); 
    if (updated) 
    {
      //Logger.log(Debug, "vehicle", "Sending metric %04X notify event", metric->id);
      metric->bleCharacteristic->notify();
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
