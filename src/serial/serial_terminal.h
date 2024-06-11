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
    uint8_t req35D[8] = {0x00, 0x03};
    uint8_t req351[8] = {0, 0, 0, 0, 0, 0, 0x22};
    uint8_t req625[6] = {0x00, 0x00, 0xFF, 0x0E, 0x20};
    uint8_t bcmVehicleStateData[8] = {0x08};
    uint8_t bmsReq[8] = {0x02, 0x21, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
};

extern SerialTerminal GlobalSerialTerminal;