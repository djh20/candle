#include "uuid.h"

uint8_t uuidBuffer[16] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00,
  0x4D, 0x91,
  0xB0, 0x49,
  0xE2, 0x82, 0x8E, 0x6D, 0xA1, 0xA0
};

BLEUUID generateUUID(uint16_t id, uint16_t discriminator) 
{
  uuidBuffer[2] = id >> 8;
  uuidBuffer[3] = id;

  uuidBuffer[4] = discriminator >> 8;
  uuidBuffer[5] = discriminator;

  return BLEUUID(uuidBuffer, 16, true);
}