#pragma once

#include <mcp2515.h>

#define DISCOVERY_MESSAGE_LIMIT 100

class CanBus 
{
  public:
    CanBus(uint8_t csPin, uint8_t intPin, CAN_SPEED bitrate);

    void begin();
    void init();
    
    void readIncomingFrame();
    void sendFlowControl(uint32_t id);
    bool sendFrame(uint32_t id, uint8_t *data, uint8_t dlc);
    void setMonitoredMessageId(uint16_t id);
    void startDiscovery();
    void stopDiscovery();
    
    bool initialized = false;

    MCP2515 *mcp;
    uint8_t csPin;
    uint8_t intPin;
    CAN_SPEED bitrate;

    // long unsigned int frameId;
    // uint8_t frameData[CAN_FRAME_MAX_DATA_LEN];
    // uint8_t frameDataLen = 0;
    
    bool discoveryInProgress = false;
    uint32_t discoveredMessages[DISCOVERY_MESSAGE_LIMIT];
    uint8_t totalDiscoveredMessages;
    
    can_frame frame;
    bool receivedFrame = false;

  private:
    can_frame txFrame;
    uint32_t lastHeartbeat = 0;
    uint8_t flowControlData[8] = {0x30, 0x00, 0x14}; // changed from 0x10 (16ms)
    uint16_t monitoredMessageId = 0xFFFE;
};
