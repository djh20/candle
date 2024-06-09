#include "poll_task.h"

PollTask::PollTask(
  CanBus *bus, int32_t interval, uint32_t timeout, uint16_t reqId, 
  uint16_t resId, uint8_t expectedResFrames, uint8_t *query, bool enabled
)
{
  this->bus = bus;
  this->interval = interval;
  this->timeout = timeout;
  this->reqId = reqId;
  this->resId = resId;
  this->expectedResFrames = expectedResFrames;
  this->enabled = enabled;
  
  memcpy(this->query, query, CAN_BUS_FRAME_DATA_LEN);
}

bool PollTask::run()
{
  if (!enabled || !bus->initialized) return false;
  if (lastRunWasSuccessful && millis() - lastSuccessMillis < interval) return false;
  
  running = true;
  lastRunMillis = millis();

  if (runLimitEnabled) runsRemaining--;

  numResponseFrames = 0;
  memset(bufferTracker, 0, expectedResFrames);
  
  log_i(
    "Sending request: %03X %02X %02X %02X %02X %02X %02X %02X %02X",
    reqId, query[0], query[1], query[2], query[3],
    query[4], query[5], query[6], query[7]
  );

  bus->mcp->sendMsgBuf(reqId, CAN_BUS_FRAME_DATA_LEN, query);

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
  return interval >= 0 && (!runLimitEnabled || runsRemaining > 0);
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

void PollTask::processFrame(uint8_t *frameData)
{
  if (!running) return;

  // The index where the incoming frame will be stored in the buffer.
  uint8_t bufferIndex = 0;

  // Only calculate buffer index if we are expecting multiple frames.
  if (expectedResFrames > 1) {
    if (frameData[0] >= query[1]) // Frames after flow control msg.
    {
      bufferIndex = frameData[0] - query[1] + 1;
    }
    else if (frameData[0] != 0x10) // 0x10 is first frame.
    {
      return; // Ignore frame.
    }
  }

  if (bufferIndex > expectedResFrames-1) return;

  log_i(
    "Response (%u): %03X %02X %02X %02X %02X %02X %02X %02X %02X",
    bufferIndex, resId, frameData[0], frameData[1],
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
  for (uint8_t i = 0; i < CAN_BUS_FRAME_DATA_LEN; i++)
  {
    buffer[bufferIndex][i] = frameData[i];
  }
  
  if (numResponseFrames >= expectedResFrames) 
  {
    success();
  }
  else if (frameData[0] == 0x10) 
  {
    bus->sendFlowControl(reqId);
  }
}