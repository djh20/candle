#include "can_bus.h"
#include <Arduino.h>

CanBus::CanBus(
  uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
  uint8_t speedset, uint8_t clockset
)
{
  this->csPin = csPin;
  this->intPin = intPin;
  this->idmodeset = idmodeset;
  this->speedset = speedset;
  this->clockset = clockset;
}

void CanBus::init() {
  pinMode(intPin, INPUT);

  mcp = new MCP_CAN(csPin);
  if (mcp->begin(idmodeset, speedset, clockset) != CAN_OK) return;
  
  mcp->setMode(MCP_NORMAL);
  initialized = true;
}

bool CanBus::readFrame() 
{
  if (digitalRead(intPin)) return false; // INT pin must be low

  memset(frameData, 0, sizeof(frameData));
  if (mcp->readMsgBuf(&frameId, &frameDataLen, frameData) != CAN_OK) return false;

  if (frameId == monitoredMessageId || monitoredMessageId == 0xFFFF)
  {
    log_i(
      "[%03X]: %02X %02X %02X %02X %02X %02X %02X %02X (%u)",
      frameId, frameData[0], frameData[1], frameData[2], frameData[3], 
      frameData[4], frameData[5], frameData[6], frameData[7], frameDataLen
    );
  }

  addFrameToCapture(frameId, frameData, frameDataLen);

  return true;
}

bool CanBus::sendFrame(uint16_t id, uint8_t *data, uint8_t dataLen)
{
  uint8_t result = mcp->sendMsgBuf(id, dataLen, data);

  if (result == CAN_OK)
  {
    addFrameToCapture(id, data, dataLen, true);
    return true;
  }
  else
  {
    log_e("Failed to send CAN frame (id: %03X)", id);
    return false;
  }
}

void CanBus::setMonitoredMessageId(uint16_t id)
{
  monitoredMessageId = id;
}

void CanBus::sendFlowControl(uint32_t frameId)
{
  uint8_t data[8] = {0x30, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
  mcp->sendMsgBuf(frameId, 8, data);
}

void CanBus::capture()
{
  memset(captureBuffer, 0, sizeof(captureBuffer));
  captureBufferIndex = 0;
  captureStartMillis = millis();
  capturing = true;
}

void CanBus::addFrameToCapture(uint16_t id, uint8_t *data, uint8_t dataLen, bool tx)
{
  if (!capturing) return;

  uint16_t ms = millis() - captureStartMillis;
  uint8_t *captureSubBuffer = captureBuffer[captureBufferIndex];
  captureSubBuffer[0] = ms >> 8;
  captureSubBuffer[1] = ms;
  captureSubBuffer[2] = id >> 8;
  captureSubBuffer[3] = id;
  captureSubBuffer[4] = (dataLen << 1) | tx;
  memcpy(captureSubBuffer+5, data, dataLen);

  if (++captureBufferIndex >= CAN_CAPTURE_BUFFER_LEN)
  {
    capturing = false;
  }
}