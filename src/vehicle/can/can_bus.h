#pragma once

#include <mcp_can.h>

class CanBus 
{
  public:
    CanBus(uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
            uint8_t speedset, uint8_t clockset);

    void init();
    bool readFrame();
    void sendFlowControl(uint32_t frameId);
    
    bool initialized = false;
    uint8_t csPin;
    uint8_t intPin;
    uint8_t idmodeset;
    uint8_t speedset;
    uint8_t clockset;

    MCP_CAN* mcp;

    long unsigned int frameId;
    byte frameData[8];
    byte frameLen = 0;
};
