#pragma once

#include <Arduino.h>

class Console
{
  public:
    void processChar(const char c);
    void processString(const char *str);

  private:
    void runCommand();
    void nextArg(char *&currentArg);
    
    char cmdBuffer[256] = {};
    uint8_t cmdBufferIndex = 0;
    bool waitingForNextArg = false;
    bool quoted = false;
};

extern Console GlobalConsole;