#include "can_bus.h"
#include <Arduino.h>

CanBus::CanBus(uint8_t csPin, uint8_t intPin, CAN_SPEED bitrate)
{
  this->csPin = csPin;
  this->intPin = intPin;
  this->bitrate = bitrate;
}

void CanBus::begin() 
{
  pinMode(intPin, INPUT);

  mcp = new MCP2515(csPin);

  // Abort function if MCP2515 fails to initialize.
  if (mcp->reset() || mcp->setBitrate(bitrate, MCP_8MHZ) || mcp->setNormalMode()) return;

  initialized = true;
}

void CanBus::readIncomingFrame()
{
  receivedFrame = false;
  
  if (!digitalRead(intPin)) // INT pin must be low
  {
    // Set all bytes in data buffer to zero. This is not necessary and is only used
    // to avoid showing data from previous frames when displaying all 8 bytes.
    memset(frame.data, 0, sizeof(frame.data));

    if (mcp->readMessage(&frame) == MCP2515::ERROR_OK)
    {
      if (frame.can_id == monitoredMessageId || monitoredMessageId == 0xFFFF)
      {
        log_i(
          "[%03X]: %02X %02X %02X %02X %02X %02X %02X %02X (%u)",
          frame.can_id, frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
          frame.data[4], frame.data[5], frame.data[6], frame.data[7], frame.can_dlc
        );
      }

      addFrameToCapture(&frame);

      receivedFrame = true;
    }
  }
}

bool CanBus::sendFrame(uint32_t id, uint8_t *data, uint8_t dlc)
{
  txFrame.can_id = id;
  txFrame.can_dlc = dlc;
  memcpy(txFrame.data, data, dlc);

  if (mcp->sendMessage(&txFrame) == MCP2515::ERROR_OK)
  {
    addFrameToCapture(&txFrame, true);
    return true;
  }
  else
  {
    log_w("Failed to verify transmission of CAN frame (id: %03X)", id);
    return false;
  }
}

void CanBus::setMonitoredMessageId(uint16_t id)
{
  monitoredMessageId = id;
}

void CanBus::sendFlowControl(uint32_t id)
{
  sendFrame(id, flowControlData, sizeof(flowControlData));
}

void CanBus::capture()
{
  memset(captureBuffer, 0, sizeof(captureBuffer));
  captureBufferIndex = 0;
  captureStartMillis = millis();
  capturing = true;
}

void CanBus::addFrameToCapture(can_frame *frame, bool tx)
{
  if (!capturing) return;

  uint16_t ms = millis() - captureStartMillis;
  uint8_t *captureSubBuffer = captureBuffer[captureBufferIndex];
  captureSubBuffer[0] = ms >> 8;
  captureSubBuffer[1] = ms;
  captureSubBuffer[2] = frame->can_id >> 8;
  captureSubBuffer[3] = frame->can_id;
  captureSubBuffer[4] = (frame->can_dlc << 1) | tx;
  memcpy(captureSubBuffer+5, frame->data, frame->can_dlc);

  if (++captureBufferIndex >= CAN_CAP_LEN)
  {
    capturing = false;
  }
}