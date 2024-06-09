#pragma once

#include <Arduino.h>
#include "can_bus.h"

#define POLL_TASK_BUFFER_LEN 8

class PollTask 
{
  public:
    PollTask(
      CanBus *bus, int32_t interval, uint32_t timeout, uint16_t reqId, 
      uint16_t resId, uint8_t expectedResFrames, uint8_t *query, bool enabled = false
    );

    bool run();
    bool isFinished();
    bool canRunAgain();

    void success();
    void cancel();
    void setEnabled(bool enabled);
    void processFrame(uint8_t *frameData);
    void setRunLimit(uint16_t limit);

    bool enabled;
    bool running = false;
    bool lastRunWasSuccessful = false;
    uint32_t lastRunMillis = 0;
    uint32_t lastSuccessMillis = 0;

    CanBus *bus;
    int32_t interval;
    uint32_t timeout;
    uint16_t reqId;
    uint16_t resId;

    uint8_t buffer[POLL_TASK_BUFFER_LEN][CAN_BUS_FRAME_DATA_LEN];

  protected:
    uint8_t expectedResFrames;
    uint8_t query[CAN_BUS_FRAME_DATA_LEN];
  
  private:
    bool bufferTracker[POLL_TASK_BUFFER_LEN];
    uint8_t numResponseFrames = 0;

    bool runLimitEnabled = false;
    uint16_t runsRemaining;
};
