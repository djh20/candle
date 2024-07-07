#pragma once

#include <Arduino.h>

#define SERIAL_CMD_MAX_LEN 256

class SerialTerminal
{
  public:
    void loop();

  private:
    void runCommand();
    void nextArg(char *&currentArg);
    
    char cmdBuffer[SERIAL_CMD_MAX_LEN] = {};
    uint8_t cmdBufferIndex = 0;
    bool waitingForNextArg = false;
    bool quoted = false;

    uint8_t emptyReq[8] = {};
    uint8_t chargePortReq[8] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x08};
    uint8_t wakeSignalReq[8] = {0x00, 0x03};
};

extern SerialTerminal GlobalSerialTerminal;