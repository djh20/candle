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
    uint8_t bmsReq[8] = {0x02, 0x21, 0x01};
    uint8_t req35D[8] = {0x00, 0x03};
    uint8_t req358[8] = {0x00, 0x08, 0x80};
    uint8_t req60D[8] = {0x08};
    uint8_t req625[6] = {0x00, 0x00, 0xFF, 0x0E, 0x20};
    uint8_t req5EB[8] = {0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t req56E[4] = {0x86};
    uint8_t req509[8] = {0xFF, 0xF8, 0xFD, 0xFD, 0x00, 0x55, 0x50, 0xFF};
    uint8_t req5E3[5] = {0xFF, 0xFF, 0xFF, 0xF0};
};

extern SerialTerminal GlobalSerialTerminal;