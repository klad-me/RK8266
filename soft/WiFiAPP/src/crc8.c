#include "crc8.h"


uint8_t CRC8(uint8_t crc, const uint8_t *data, uint8_t len)
{
    while (len--)
    {
	uint8_t i;
	
	crc^=*data++;
	for (i=0; i<8; i++)
	    crc=(crc << 1) ^ ( (crc & 0x80) ? 0x2F : 0x00 );
    }
    
    return crc;
}
