#pragma once

#include <Arduino.h>
#include "can_bus.h"

#define POLL_TASK_BUFFER_LEN 8

class PollTask 
{
  public:
    PollTask(CanBus *bus, uint32_t interval, uint32_t timeout, uint32_t requestId, uint32_t responseId, uint8_t responseFrames, uint8_t *query);

    void run();
    void success();
    void cancel();
    bool processFrame(uint8_t *frameData);
  
    CanBus *bus;
    uint32_t interval;
    uint32_t timeout;
    uint32_t requestId;
    uint32_t responseId;
    uint8_t expectedResponseFrames;
    uint8_t *query;

    uint8_t buffer[POLL_TASK_BUFFER_LEN][CAN_BUS_MAX_FRAME_LEN];
    bool bufferTracker[POLL_TASK_BUFFER_LEN];
    uint8_t numResponseFrames = 0;

    bool running = false;
    uint32_t lastRunMillis = 0;
    uint32_t nextRunMillis = 0;
};
