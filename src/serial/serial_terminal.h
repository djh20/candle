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

    uint8_t emptyReq[8] = {};
    uint8_t chargePortReq[8] = {0x00, 0x03, 0x00, 0x00, 0x00, 0x08};
    uint8_t wakeSignalReq[8] = {0x00, 0x03};
};

extern SerialTerminal GlobalSerialTerminal;