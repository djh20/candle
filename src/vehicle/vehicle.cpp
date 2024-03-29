#include "vehicle.h"
#include <Arduino.h>
#include "../utils/logger.h"
#include "metric/command_callbacks.h"
#include <BLE2902.h>

Vehicle::Vehicle() {}

void Vehicle::init(BLEServer *bleServer)
{
  this->bleServer = bleServer;
  bleService = bleServer->createService(BLEUUID(METRICS_SERVICE_UUID), 128U, 0);

  BLECharacteristic *commandCharacteristic = new BLECharacteristic(
    BLEUUID(COMMAND_CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  commandCharacteristic->setCallbacks(new CommandCallbacks(this));

  bleService->addCharacteristic(commandCharacteristic);

  registerMetric(awake = new MetricInt(METRIC_AWAKE, Unit::None));
  registerMetric(tripDistance = new MetricFloat(METRIC_TRIP_DISTANCE, Unit::Kilometers, Precision::High));

  registerAll();
  
  for (uint8_t i = 0; i < totalMetrics;) {
    BLECharacteristic *characteristic = new BLECharacteristic(
      BLEUUID((uint16_t)totalMetricCharacteristics),
      BLECharacteristic::PROPERTY_READ |
      BLECharacteristic::PROPERTY_NOTIFY
    );
    uint8_t characteristicValueIndex = 0;

    BLEDescriptor *descriptor = new BLEDescriptor(BLEUUID((uint16_t)DESCRIPTOR_UUID), MAX_DESCRIPTOR_VALUE_SIZE);
    uint8_t descriptorBuffer[MAX_DESCRIPTOR_VALUE_SIZE];
    uint8_t descriptorBufferSize = 0;
  
    for (; i < totalMetrics; i++) {
      Metric *metric = metrics[i];
      int32_t val =32;
      characteristic->setValue(val);

      // TODO: Someow hget data size without explicitly writing method for it.
      uint8_t characteristicValueSize = characteristicValueIndex + metric->getDataSize();

      if (characteristicValueSize > MAX_CHARACTERISTIC_VALUE_SIZE)
        break;

      metric->getDescriptorData(descriptorBuffer, descriptorBufferSize, characteristicValueIndex);
      characteristicValueIndex = characteristicValueSize;
    }

    descriptor->setValue(descriptorBuffer, descriptorBufferSize);

    characteristic->addDescriptor(descriptor);
    characteristic->addDescriptor(new BLE2902());

    registerCharacteristic(characteristic);
  }

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

  //bleService->addCharacteristic(metric->bleCharacteristic);
  
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

void Vehicle::registerCharacteristic(BLECharacteristic *characteristic) {
  metricCharacteristics[totalMetricCharacteristics++] = characteristic;
  bleService->addCharacteristic(characteristic);

  Logger.log(Info, "vehicle", "Registered characteristic %s", characteristic->getUUID().toString().c_str());
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
  uint8_t characteristicIndex = 0;

  for (uint8_t i = 0; i < totalMetrics;) {
    BLECharacteristic *characteristic = metricCharacteristics[characteristicIndex];
    uint8_t characteristicValueIndex = 0;
    bool updated = false;

    for (; i < totalMetrics; i++) {
      Metric *metric = metrics[i];
      uint8_t characteristicValueSize = characteristicValueIndex + metric->getDataSize();

      if (characteristicValueSize > MAX_CHARACTERISTIC_VALUE_SIZE)
        break;
      
      metric->getValueData(characteristicValueBuffer, characteristicValueIndex);
      if (metric->lastUpdateMillis >= sinceMillis) updated = true;
    }
    
    if (updated) {
      characteristic->setValue(characteristicValueBuffer, characteristicValueIndex);
      characteristic->notify();
    }
    characteristicIndex++;
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
