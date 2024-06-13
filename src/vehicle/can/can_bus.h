#pragma once

#include <mcp_can.h>

#define CAN_FRAME_MAX_DATA_LEN 8
#define CAN_CAPTURE_BUFFER_LEN 30

class CanBus 
{
  public:
    CanBus(uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
            uint8_t speedset, uint8_t clockset);

    void init();
    bool readFrame();
    void sendFlowControl(uint32_t frameId);
    void capture();
    bool sendFrame(uint16_t id, uint8_t *data, uint8_t dataLen);
    void setMonitoredMessageId(uint16_t id);
    
    bool initialized = false;
    uint8_t csPin;
    uint8_t intPin;
    uint8_t idmodeset;
    uint8_t speedset;
    uint8_t clockset;

    MCP_CAN* mcp;

    long unsigned int frameId;
    uint8_t frameData[CAN_FRAME_MAX_DATA_LEN];
    uint8_t frameDataLen = 0;

    bool capturing = false;
    uint32_t captureStartMillis;
    uint8_t captureBuffer[CAN_CAPTURE_BUFFER_LEN][CAN_FRAME_MAX_DATA_LEN+5];
    uint8_t captureBufferIndex;

  private:
    void addFrameToCapture(uint16_t id, uint8_t *data, uint8_t dataLen, bool tx = false);

    uint16_t monitoredMessageId = 0xFFFE;
};
