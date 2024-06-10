#include "poll_task.h"

PollTask::PollTask(
  CanBus *bus, uint16_t reqId, uint8_t *reqData, uint8_t reqDataLen
)
{
  this->bus = bus;
  this->reqId = reqId;
  this->reqDataLen = reqDataLen;
  
  memset(this->reqData, 0, sizeof(this->reqData));
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

void PollTask::configureResponse(uint16_t id, uint8_t totalFrames)
{
  if (totalFrames == 0 || expectedResFrameCount > 0) return;

  resId = id;
  expectedResFrameCount = totalFrames;

  resBuffer = new uint8_t*[totalFrames];
  for (uint8_t i = 0; i < totalFrames; i++)
  {
    resBuffer[i] = new uint8_t[CAN_FRAME_MAX_DATA_LEN];
  }
}

void PollTask::setInterval(uint32_t interval)
{
  this->interval = interval;
}

void PollTask::setTimeout(uint32_t timeout)
{
  this->timeout = timeout;
}

bool PollTask::run()
{
  if (!enabled || !bus->initialized) return false;
  if (lastRunWasSuccessful && millis() - lastSuccessMillis < interval) return false;
  
  running = true;
  lastRunMillis = millis();

  if (runLimitEnabled) runsRemaining--;

  currentResFrameCount = 0;
  resBufferTracker = 0;
  
  log_i(
    "Request: %03X %02X %02X %02X %02X %02X %02X %02X %02X",
    reqId, reqData[0], reqData[1], reqData[2], reqData[3],
    reqData[4], reqData[5], reqData[6], reqData[7]
  );

  bus->mcp->sendMsgBuf(reqId, reqDataLen, reqData);

  return true;
}

bool PollTask::isFinished()
{
  if (!running) return true;

  if (millis() - lastRunMillis >= timeout)
  {
    cancel();
    return true;
  }

  return false;
}

bool PollTask::canRunAgain()
{
  return !runLimitEnabled || runsRemaining > 0;
}

void PollTask::success()
{
  lastRunWasSuccessful = true;
  lastSuccessMillis = millis();
  running = false;
}

void PollTask::cancel()
{
  lastRunWasSuccessful = false;
  running = false;
}

void PollTask::setEnabled(bool enabled)
{
  this->enabled = enabled;
}

void PollTask::setRunLimit(uint16_t limit)
{
  runsRemaining = limit;
  runLimitEnabled = true;
}

void PollTask::waitUntilNextInterval()
{
  lastRunWasSuccessful = true;
  lastSuccessMillis = millis();
}

void PollTask::processFrame(uint8_t *frameData, uint8_t frameDataLen)
{
  if (!running || expectedResFrameCount == 0) return;

  // The index where the incoming frame will be stored in the buffer.
  uint8_t bufferIndex = 0;

  // Only calculate buffer index if we are expecting multiple frames.
  if (expectedResFrameCount > 1) {
    if (frameData[0] >= reqData[1]) // Frames after flow control msg.
    {
      bufferIndex = frameData[0] - reqData[1] + 1;
    }
    else if (frameData[0] != 0x10) // 0x10 is first frame.
    {
      return; // Ignore frame.
    }
  }
  
  if (bufferIndex >= expectedResFrameCount) return;

  log_i(
    "Response (%u): %03X %02X %02X %02X %02X %02X %02X %02X %02X",
    bufferIndex, resId, frameData[0], frameData[1],
    frameData[2], frameData[3], frameData[4],
    frameData[5], frameData[6], frameData[7]
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
  for (uint8_t i = 0; i < CAN_FRAME_MAX_DATA_LEN; i++)
  {
    resBuffer[bufferIndex][i] = frameData[i];
  }
  
  if (currentResFrameCount >= expectedResFrameCount) 
  {
    success();
  }
  else if (frameData[0] == 0x10) 
  {
    bus->sendFlowControl(reqId);
    log_i("Sent flow control frame");
  }
}