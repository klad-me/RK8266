#ifndef TAPE_H
#define TAPE_H


#include "ets.h"


void tape_init(void);
void tape_in(void);
bool tape_out(void);
bool tape_periodic(void);

void tape_save(void);
void tape_load(uint16_t n);


#endif
