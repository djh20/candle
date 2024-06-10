#pragma once

#include <Arduino.h>

#define SERIAL_CMD_ID_LEN 3U
#define SERIAL_CMD_ARGS_LEN 32U

class SerialTerminal
{
  public:
    void loop();

  private:
    void runCommand();

    char cmdId[SERIAL_CMD_ID_LEN];
    uint16_t cmdArgs[SERIAL_CMD_ARGS_LEN];
    uint8_t cmdCharIndex = 0;
    uint8_t cmdArgIndex = 0;
    bool cmdArgReceived = false;

    uint8_t emptyData[8] = {};
};

extern SerialTerminal GlobalSerialTerminal;