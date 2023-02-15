#include "logger.h"
#include <Arduino.h>

/*
const char *LOG_LEVEL_PREFIXES[] = {
  "ERROR",
  "WARN",
  "info",
  "debug"
};
*/

const char LOG_LEVEL_SYMBOLS[] = {
  'E',
  'W',
  'I',
  'D'
};

SerialLogger::SerialLogger() {}

void SerialLogger::log(LogLevel level, const char *topic, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  memset(msgBuffer, 0, LOGGER_BUFFER_SIZE);
  vsprintf(msgBuffer, format, args);

  uint32_t now = millis();
  Serial.printf("%c  %-10u  %-10s  %s\n", LOG_LEVEL_SYMBOLS[level], now, topic, msgBuffer);

  va_end(args);
}

SerialLogger Logger;
