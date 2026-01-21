#ifndef COMMUNICATION_CRC16_H_
#define COMMUNICATION_CRC16_H_

#include <stdint.h>
#include <stddef.h>

uint16_t crc16(uint8_t const *data, size_t size);
void crc16_fillTable();

#endif /* COMMUNICATION_CRC16_H_ */
