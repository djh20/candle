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

      receivedFrame = true;

      if (discoveryInProgress)
      {
        bool newMessageDiscovered = true;

        for (uint8_t i = 0; i < totalDiscoveredMessages; i++)
        {
          if (discoveredMessages[i] == frame.can_id)
          {
            newMessageDiscovered = false;
            break;
          }
        }

        if (newMessageDiscovered)
        {
          discoveredMessages[totalDiscoveredMessages++] = frame.can_id;
          log_i("Message discovered: [%03X]", frame.can_id);

          if (totalDiscoveredMessages >= DISCOVERY_MESSAGE_LIMIT) stopDiscovery();
        }
      }
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
    log_i(
      "TX: %03X %02X %02X %02X %02X %02X %02X %02X %02X (%u)",
      id, data[0], data[1], data[2], data[3],
      data[4], data[5], data[6], data[7], dlc
    );
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

void CanBus::startDiscovery()
{
  discoveryInProgress = true;
  totalDiscoveredMessages = 0;
  log_i("Discovery started");
}

void CanBus::stopDiscovery()
{
  discoveryInProgress = false;
  log_i("Discovery stopped");
}

void CanBus::sendFlowControl(uint32_t id)
{
  sendFrame(id, flowControlData, sizeof(flowControlData));
}