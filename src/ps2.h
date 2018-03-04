#ifndef PS2_DRV_H
#define PS2_DRV_H


#include <os_type.h>
#include "ps2_codes.h"


#ifdef __cplusplus
extern "C" {
#endif


void ps2_init(void);
void ps2_periodic(void);

uint16_t ps2_read(void);
void ps2_leds(bool caps, bool num, bool scroll);


#ifdef __cplusplus
};
#endif


#endif
