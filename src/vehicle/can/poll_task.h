#pragma once

#include <Arduino.h>
#include "can_bus.h"

#define POLL_TASK_BUFFER_LEN 8

class PollTask 
{
  public:
    PollTask(
      CanBus *bus, int32_t interval, uint32_t timeout, uint32_t reqId, 
      uint32_t resId, uint8_t expectedResFrames, uint8_t *query, uint8_t queryLen = 8,
      bool enabled = false
    );

    bool run();
    bool isFinished();

    void success();
    void cancel();
    void setEnabled(bool enabled);
    void processFrame(uint8_t *frameData);

    bool enabled;
    bool running = false;
    bool lastRunWasSuccessful = false;
    uint32_t lastRunMillis = 0;
    uint32_t lastSuccessMillis = 0;

    CanBus *bus;
    int32_t interval;
    uint32_t timeout;
    uint32_t reqId;
    uint32_t resId;

    uint8_t buffer[POLL_TASK_BUFFER_LEN][CAN_BUS_FRAME_DATA_LEN];

  protected:
    uint8_t expectedResFrames;
    uint8_t query[CAN_BUS_FRAME_DATA_LEN];
    uint8_t queryLen;
  
  private:
    bool bufferTracker[POLL_TASK_BUFFER_LEN];
    uint8_t numResponseFrames = 0;
};
