#pragma once

#define LOGGER_BUFFER_SIZE 256

enum LogLevel 
{
  Error,
  Warn,
  Info,
  Debug
};

class SerialLogger 
{
  public:
    SerialLogger();

    void log(LogLevel level, const char *topic, const char *format, ...);
  
  private:
    char msgBuffer[LOGGER_BUFFER_SIZE];
};

extern SerialLogger Logger;
