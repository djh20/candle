#include "can_bus.h"
#include <Arduino.h>

CanBus::CanBus(uint8_t id, uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
                uint8_t speedset, uint8_t clockset)
{
  this->id = id;
  this->csPin = csPin;
  this->intPin = intPin;
  this->idmodeset = idmodeset;
  this->speedset = speedset;
  this->clockset = clockset;
}

bool CanBus::init() {
  mcp = new MCP_CAN(csPin);
  if (mcp->begin(idmodeset, speedset, clockset) != CAN_OK) return false;
  
  mcp->setMode(MCP_NORMAL);
  return true;
}

bool CanBus::readFrame() 
{
  if (digitalRead(intPin)) return false;

  mcp->readMsgBuf(&frameId, &frameLen, frameData);
  return true;
}
