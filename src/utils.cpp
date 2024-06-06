#include "utils.h"

uint8_t Utils::hexCharToInt(char hexChar)
{
  return (hexChar >= 'A') ? (hexChar - 'A' + 10) : (hexChar - '0');
}