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
};

extern SerialTerminal GlobalSerialTerminal;