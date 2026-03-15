#include "crc16.h"

uint16_t crc16(uint8_t const *data, size_t size)
{
	uint16_t crc = 0xFFFF;
	char i = 0;

	while(size--)
	{
		crc ^= (*data++);

		for(i = 0; i < 8; i++)
		{
			if( crc & 1 )
			{
				crc >>= 1;
				crc ^= 0xA001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}

	return crc;
}
