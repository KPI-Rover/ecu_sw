#include "crc16.h"

uint16_t crc16(uint8_t const *data, size_t size) {
  uint16_t crc = 0;
  while (size--) {
    crc ^= *data++;
    for (unsigned k = 0; k < 8; k++)
      crc = crc & 1 ? (crc >> 1) ^ 0x8005 : crc >> 1;
  }
  return crc;
}
