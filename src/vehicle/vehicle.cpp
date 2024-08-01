#include "vehicle.h"
#include "metric/metric_manager.h"

#define TEST_CYCLE_INTERVAL 200U

Vehicle::Vehicle(const char *domain)
{
  this->domain = domain;
}

void Vehicle::begin()
{
  registerMetric(ignition = new IntMetric<1>(domain, "ignition", MetricType::Statistic));
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
  task->setCallbacks(this);
  log_i("Registered task: [%s]", task->id);
}

void Vehicle::setTaskInterval(Task *task, uint32_t interval)
{
  for (uint8_t i = 0; i < totalTasks; i++)
  {
    if (task == tasks[i])
    {
      log_i("Set interval of task [%s] to %u ms", task->id, interval);
      taskIntervals[i] = interval;
      return;
    }
  }
  log_e("Failed to set interval of task [%s] (not found)", task->id);
}

void Vehicle::clearTaskInterval(Task *task)
{
  setTaskInterval(task, 0);
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

bool Vehicle::isTaskInQueue(Task *task)
{
  for (uint8_t i = 0; i < totalTasksInQueue; i++)
  {
    if (taskQueue[i] == task) return true;
  }

  return false;
}

void Vehicle::runHomeTasks() {}

void Vehicle::handleTasks()
{
  uint32_t now = millis();

  for (uint8_t i = 0; i < totalTasks; i++)
  {
    Task *task = tasks[i];
    uint32_t interval = taskIntervals[i];

    if (interval != 0 && (now - task->lastFinishMillis >= interval || task->yetToRun))
    {
      if (!isTaskInQueue(task)) runTask(task);
    }
  }

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
void Vehicle::onTaskRun(Task *task) {}
void Vehicle::onPollResponse(Task *task, uint8_t **frames) {}
void Vehicle::updateExtraMetrics() {}
void Vehicle::metricUpdated(Metric *metric) {}
void Vehicle::testCycle() {}