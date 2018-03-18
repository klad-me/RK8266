#ifndef KEYMAP_H
#define KEYMAP_H


#ifdef __cplusplus
extern "C" {
#endif


#include "ets.h"


void keymap_init(void);
uint16_t keymap_periodic(void);


#ifdef __cplusplus
};
#endif


#endif
