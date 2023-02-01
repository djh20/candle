#ifndef _CAN_BUS_H_
#define _CAN_BUS_H_

#include <mcp_can.h>

class CanBus 
{
  public:
    CanBus(uint8_t id, uint8_t csPin, uint8_t intPin, uint8_t idmodeset, 
            uint8_t speedset, uint8_t clockset);

    bool init();
    bool readFrame();
    
    uint8_t id;
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

#endif
