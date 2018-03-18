#ifndef CRC8_H
#define CRC8_H


#ifdef __ets__
    #include "ets.h"
#else
    #include <c_types.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


#define CRC8_INIT	0xFF
#define CRC8_OK		0x00


uint8_t CRC8(uint8_t crc, const uint8_t *data, uint8_t len);


#ifdef __cplusplus
}
#endif


#endif
