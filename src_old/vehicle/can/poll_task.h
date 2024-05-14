#pragma once

#include <Arduino.h>
#include "can_bus.h"

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
    uint8_t responseFrames;
    uint8_t *query;

    uint8_t bufferIndex;
    uint8_t buffer[8][8];

    bool running = false;
    uint32_t lastRunMillis = 0;
    uint32_t nextRunMillis = 0;
};
