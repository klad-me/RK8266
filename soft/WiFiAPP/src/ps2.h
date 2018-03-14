#ifndef PS2_DRV_H
#define PS2_DRV_H


#include <os_type.h>
#include "ps2_codes.h"


#ifdef __cplusplus
extern "C" {
#endif


void ps2_init(void);
uint16_t ps2_read(void);


#ifdef __cplusplus
};
#endif


#endif
