#include "vehicle.h"
#include "metric/metric_manager.h"

#define TEST_CYCLE_INTERVAL 200U

Vehicle::Vehicle(const char *domain)
{
  this->domain = domain;
}

void Vehicle::begin()
{
  registerMetric(ignition = new IntMetric(domain, "ignition", MetricType::Statistic));
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
  buses[totalBuses++] = bus;

  bus->begin();

  if (!bus->initialized) {
    log_e("Failed to initialize bus %u", totalBuses-1);
    return;
  }

  log_i("Registered bus %u", totalBuses-1);
}

void Vehicle::registerMetric(Metric *metric) 
{
  metric->onUpdate([this, metric]() {
    metricUpdated(metric);
  });

  GlobalMetricManager.registerMetric(metric);
}

void Vehicle::registerMetrics(std::initializer_list<Metric*> metrics)
{
  for (Metric *metric : metrics)
  {
    registerMetric(metric);
  }
}

void Vehicle::registerTask(Task *task)
{
  tasks[totalTasks++] = task;
  log_i("Registered task: %s", task->id);
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
  }
}

void Vehicle::runTask(Task *task)
{
  taskQueue[totalTasksInQueue++] = task;
}

Task *Vehicle::getTask(const char *id)
{
  for (uint8_t i = 0; i < totalTasks; i++)
  {
    Task *task = tasks[i];
    if (strcmp(id, task->id) == 0)
    {
      return task;
    }
  }
  
  return nullptr;
}

void Vehicle::handleTasks()
{
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
      // Move queue forward.
      for (uint8_t i = 0; i < totalTasksInQueue; i++)
      {
        taskQueue[i] = taskQueue[i+1];
      }

      totalTasksInQueue--;

      currentTask = NULL;
    }
  }
}

void Vehicle::processFrame(CanBus *bus, const uint32_t &id, uint8_t *data) {}
// void Vehicle::processPollResponse(CanBus *bus, PollTask *task, uint8_t **frames) {}
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
void Vehicle::testCycle() {}