#include "poll_task.h"
#include "../../utils/logger.h"

PollTask::PollTask(CanBus *bus, uint32_t interval, uint32_t timeout, uint32_t requestId, uint32_t responseId, uint8_t responseFrames, uint8_t *query)
{
  this->bus = bus;
  this->interval = interval;
  this->timeout = timeout;
  this->requestId = requestId;
  this->responseId = responseId;
  this->responseFrames = responseFrames;
  this->query = query;
}

void PollTask::run()
{
  running = true;
  bufferIndex = 0;
  lastRunMillis = millis();
  if (bus->initialized) {
    Logger.log(Debug, "task", "Sending query %X %X %X %X %X %X %X %X", query[0], query[1], query[2], query[3], query[4], query[5], query[6], query[7]);
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
  for (uint8_t i = 0; i < 8; i++)
  {
    buffer[bufferIndex][i] = frameData[i];
  }

  Logger.log(
    Debug,
    "task",
    "Response (%u): %X %X %X %X %X %X %X %X",
    bufferIndex, frameData[0], frameData[1], frameData[2], frameData[3], frameData[4], frameData[5], frameData[6], frameData[7]
  );

  if (bufferIndex >= responseFrames-1)
  {
    success();
    return true;
  }
  else if (bufferIndex == 0) 
  {
    bus->sendFlowControl(requestId);
  }

  bufferIndex++;

  return false;
}
