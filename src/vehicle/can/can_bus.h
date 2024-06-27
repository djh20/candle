#pragma once

#include <mcp2515.h>

#define CAN_CAP_LEN 30

class CanBus 
{
  public:
    CanBus(uint8_t csPin, uint8_t intPin, CAN_SPEED bitrate);

    void begin();
    
    void readIncomingFrame();
    void sendFlowControl(uint32_t id);
    void capture();
    bool sendFrame(uint32_t id, uint8_t *data, uint8_t dlc);
    void setMonitoredMessageId(uint16_t id);
    
    bool initialized = false;

    MCP2515 *mcp;
    uint8_t csPin;
    uint8_t intPin;
    CAN_SPEED bitrate;

    // long unsigned int frameId;
    // uint8_t frameData[CAN_FRAME_MAX_DATA_LEN];
    // uint8_t frameDataLen = 0;

    bool capturing = false;
    uint32_t captureStartMillis;
    uint8_t captureBuffer[CAN_CAP_LEN][CAN_MAX_DLEN+5];
    uint8_t captureBufferIndex;
    
    can_frame frame;
    bool receivedFrame = false;

  private:
    void addFrameToCapture(can_frame *frame, bool tx = false);

    can_frame txFrame;
    uint8_t flowControlData[8] = {0x30, 0x00, 0x14}; // changed from 0x10 (16ms)
    uint16_t monitoredMessageId = 0xFFFE;
};
