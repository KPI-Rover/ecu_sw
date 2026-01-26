#include "crc16.h"

uint16_t crc_table[UINT8_MAX];

uint16_t crc16(uint8_t const *data, size_t size) {
	uint16_t crc = 0;
	while (size--) {
		/* XOR-in next input byte into MSB of crc, that's our new intermediate dividend */
		uint8_t pos = (uint8_t) ((crc >> 8) ^ *data++); /* equal: ((crc ^ (b << 8)) >> 8) */
		/* Shift out the MSB used for division per lookuptable and XOR with the remainder */
		crc = (uint16_t) ((crc << 8) ^ (uint16_t) (crc_table[pos]));
	}
	return crc;
}

void crc16_fillTable() {
	const uint16_t generator = 0x8005;
	for (int dividend = 0; dividend < UINT8_MAX; ++dividend) {
		uint16_t current_byte = dividend << 8;

		for (uint8_t bit = 0; bit < 8; ++bit) {
			if ((current_byte & 0x8000) != 0) {
				current_byte <<= 1;
				current_byte ^= generator;
			} else {
				current_byte <<= 1;
			}
		}
		crc_table[dividend] = current_byte;
	}
}
