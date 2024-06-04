#include "poll_task.h"

PollTask::PollTask(CanBus *bus, uint32_t interval, uint32_t timeout, uint32_t requestId, uint32_t responseId, uint8_t responseFrames, uint8_t *query)
{
  this->bus = bus;
  this->interval = interval;
  this->timeout = timeout;
  this->requestId = requestId;
  this->responseId = responseId;
  this->expectedResponseFrames = responseFrames;
  this->query = query;
}

void PollTask::run()
{
  running = true;
  lastRunMillis = millis();
  if (bus->initialized) {
    numResponseFrames = 0;
    memset(bufferTracker, 0, expectedResponseFrames);

    log_d("Sending query %X %X %X %X %X %X %X %X", query[0], query[1], query[2], query[3], query[4], query[5], query[6], query[7]);
    bus->mcp->sendMsgBuf(requestId, 8, query);
    
  } else {
    cancel();
  }
}

void PollTask::success()
{
  running = false;
  nextRunMillis = millis() + interval;
}

void PollTask::cancel()
{
  running = false;
}

bool PollTask::processFrame(uint8_t *frameData)
{
  // The index where the incoming frame will be stored in the buffer.
  uint8_t bufferIndex = 0;

  // Only calculate buffer index if we are expecting multiple frames.
  if (expectedResponseFrames > 1) {
    if (frameData[0] >= query[1]) // Frames after flow control msg.
    {
      bufferIndex = frameData[0] - query[1] + 1;
    }
    else if (frameData[0] != 0x10) // 0x10 is first frame.
    {
      return false; // Ignore frame.
    }
  }

  log_d(
    "Response (%u): %X %X %X %X %X %X %X %X",
    bufferIndex, frameData[0], frameData[1],
    frameData[2], frameData[3], frameData[4],
    frameData[5], frameData[6], frameData[7]
  );

  // Keep track of which areas of the buffer have been filled.
  // This is necessary because new frames can overwrite old frames 
  // if additional responses are received due to another request
  // happening on the bus as the same time.
  if (bufferTracker[bufferIndex] == false) {
    bufferTracker[bufferIndex] = true;
    numResponseFrames++;
  }

  // Copy frame data to buffer at specified index.
  for (uint8_t i = 0; i < CAN_BUS_MAX_FRAME_LEN; i++)
  {
    buffer[bufferIndex][i] = frameData[i];
  }
  
  if (numResponseFrames >= expectedResponseFrames) 
  {
    success();
    return true;
  }
  else if (frameData[0] == 0x10) {
    bus->sendFlowControl(requestId);
  }

  return false;
}
