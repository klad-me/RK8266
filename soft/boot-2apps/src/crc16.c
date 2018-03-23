#include "crc16.h"


uint16_t CRC16(uint16_t crc, const uint8_t *data, uint16_t size)
{
    uint16_t i;
    uint8_t x,cnt;
    
    for (i=0; i<size; i++)
    {
        x=data[i];
        for (cnt=0; cnt<8; cnt++)
        {
            if ((x^crc)&1) crc=((crc^0x4002)>>1)|0x8000; else
                           crc=crc>>1;
            x>>=1;
        }
    }

    return crc;
}
