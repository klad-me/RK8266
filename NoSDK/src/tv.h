#ifndef TV_H
#define TV_H


#include "ets.h"


#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t tv_empty_line[80];


void tv_init(void);
void tv_start(void);


#ifdef __cplusplus
}
#endif


#endif
