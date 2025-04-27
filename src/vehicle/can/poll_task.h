#pragma once

#include <Arduino.h>
#include "can_bus.h"
#include "task.h"

class PollTask: public Task
{
  public:
    PollTask(const char *id, CanBus *bus, uint32_t reqId, const uint8_t reqData[], uint8_t reqDataLen);
    ~PollTask();
    
    void configureResponse(uint32_t id, uint8_t totalFrames);
  
  protected:
    void preRun() override;
    void initiateAttempt() override;
    void process() override;
  
  private:
    CanBus *bus;

    uint32_t reqId;
    uint8_t reqData[CAN_MAX_DLEN] = {};
    uint8_t reqDataLen;

    uint32_t resId;
    uint8_t expectedResFrameCount = 0;
    uint8_t currentResFrameCount = 0;

    uint8_t **resBuffer;
    uint8_t resBufferTracker = 0;
};