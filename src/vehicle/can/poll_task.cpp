#include "poll_task.h"

PollTask::PollTask(
  const char *id, CanBus *bus, uint32_t reqId, const uint8_t reqData[], uint8_t reqDataLen
) : Task(id) {
  this->bus = bus;
  this->reqId = reqId;
  this->reqDataLen = reqDataLen;

  memcpy(this->reqData, reqData, reqDataLen);
}

PollTask::~PollTask()
{
  if (expectedResFrameCount == 0) return;

  // Free each sub-array
  for(uint8_t i = 0; i < expectedResFrameCount; i++) {
    delete[] resBuffer[i];
  }

  delete[] resBuffer;
}

void PollTask::preRun()
{
  // We only reset these values on each run so that received data is retained
  // across attempts (useful for unreliable polling requests).
  currentResFrameCount = 0;
  resBufferTracker = 0;
}

void PollTask::initiateAttempt()
{
  bus->sendFrame(reqId, reqData, reqDataLen);

  if (expectedResFrameCount == 0)
  {
    endAttempt(true);
  }
}

void PollTask::process()
{
  if (!bus->receivedFrame || bus->frame.can_id != resId) return;
  
  uint8_t *data = bus->frame.data;
  uint8_t dataLen = bus->frame.can_dlc;

  // The index where the incoming frame will be stored in the buffer.
  uint8_t bufferIndex = 0;

  // Only calculate buffer index if we are expecting multiple frames.
  if (expectedResFrameCount > 1) {
    if (data[0] >= reqData[1]) // Frames after flow control msg.
    {
      bufferIndex = data[0] - reqData[1] + 1;
    }
    else if (data[0] != 0x10) // 0x10 is first frame.
    {
      return; // Ignore frame.
    }
  }
  
  if (bufferIndex >= expectedResFrameCount) return;

  log_i(
    "Response #%u: %03X %02X %02X %02X %02X %02X %02X %02X %02X (%u)",
    bufferIndex+1, resId, data[0], data[1],data[2], data[3], data[4], data[5],
    data[6], data[7], dataLen
  );

  // Keep track of which areas of the buffer have been filled.
  // This is necessary because new response frames can overwrite old frames if
  // they have the same first byte / buffer index, but we still need to know
  // the actual frame count.
  if (((resBufferTracker >> bufferIndex) & 0x01) == 0) {
    resBufferTracker |= (1 << bufferIndex);
    currentResFrameCount++;
  }

  // Copy frame data to buffer at specified index.
  for (uint8_t i = 0; i < dataLen; i++)
  {
    resBuffer[bufferIndex][i] = data[i];
  }
  
  if (currentResFrameCount >= expectedResFrameCount) 
  {
    if (callbacks) 
    {
      endAttempt(callbacks->onPollResponse(this, resBuffer));
    }
    else 
    {
      endAttempt(true);
    }
  }
  else if (data[0] == 0x10) 
  {
    bus->sendFlowControl(reqId);
    log_i("Sent flow control frame");
  }
}

void PollTask::configureResponse(uint32_t id, uint8_t totalFrames)
{
  if (totalFrames == 0 || expectedResFrameCount > 0) return;

  resId = id;
  expectedResFrameCount = totalFrames;

  resBuffer = new uint8_t*[totalFrames];
  for (uint8_t i = 0; i < totalFrames; i++)
  {
    resBuffer[i] = new uint8_t[CAN_MAX_DLEN];
  }
}