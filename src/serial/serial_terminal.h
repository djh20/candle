#pragma once

#define SERIAL_CMD_ID_LEN 3
#define SERIAL_CMD_DATA_LEN 64

class SerialTerminal
{
  public:
    void begin();
    void loop();

  private:
    void runCommand();

    char commandId[SERIAL_CMD_ID_LEN];
    uint8_t commandData[SERIAL_CMD_DATA_LEN];
    uint8_t commandPos = 0;
};

extern SerialTerminal GlobalSerialTerminal;