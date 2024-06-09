#include "can_bus.h"
#include <Arduino.h>

CanBus::CanBus(uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
                uint8_t speedset, uint8_t clockset)
{
  this->csPin = csPin;
  this->intPin = intPin;
  this->idmodeset = idmodeset;
  this->speedset = speedset;
  this->clockset = clockset;
}

void CanBus::init() {
  mcp = new MCP_CAN(csPin);
  if (mcp->begin(idmodeset, speedset, clockset) != CAN_OK) return;
  
  mcp->setMode(MCP_NORMAL);
  initialized = true;
}

bool CanBus::readFrame() 
{
  if (digitalRead(intPin)) return false;

  memset(frameData, 0, sizeof(frameData));
  mcp->readMsgBuf(&frameId, &frameLen, frameData);
  return true;
}

void CanBus::sendFlowControl(uint32_t frameId)
{
  uint8_t data[8] = {0x30, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
  mcp->sendMsgBuf(frameId, 8, data);
}
