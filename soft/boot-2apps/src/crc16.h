#ifndef LIB_CRC16_H
#define LIB_CRC16_H


#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


#define CRC16_INIT	0xFFFF
#define CRC16_OK	0x0000


uint16_t CRC16(uint16_t crc, const uint8_t *data, uint16_t size);


#ifdef __cplusplus
}
#endif


#endif
