#pragma once

#include <Arduino.h>
#include "can_bus.h"

#define POLL_TASK_BUFFER_LEN 8

class PollTask 
{
  public:
    PollTask(
      CanBus *bus, uint16_t reqId, uint8_t *reqData, uint8_t reqDataLen
    );

    ~PollTask();

    void configureResponse(uint16_t id, uint8_t totalFrames);
    void setInterval(uint32_t interval);
    void setTimeout(uint32_t timeout);

    bool run();
    bool isFinished();
    bool canRunAgain();

    void success();
    void cancel();
    void setEnabled(bool enabled);
    void processFrame(uint8_t *frameData, uint8_t frameDataLen);
    void setRunLimit(uint16_t limit);
    void waitUntilNextInterval();

    bool enabled = false;
    bool running = false;
    bool lastRunWasSuccessful = false;
    uint32_t lastRunMillis = 0;
    uint32_t lastSuccessMillis = 0;

    CanBus *bus;
    uint32_t interval = 0;
    uint32_t timeout = 0;
    uint16_t reqId;
    uint16_t resId = 0x0000;

    uint8_t **resBuffer;

  private:
    uint8_t reqData[CAN_FRAME_MAX_DATA_LEN];
    uint8_t reqDataLen;

    uint8_t expectedResFrameCount = 0;
    uint8_t currentResFrameCount = 0;
    
    uint8_t resBufferTracker = 0;
    
    bool runLimitEnabled = false;
    uint16_t runsRemaining;
};
