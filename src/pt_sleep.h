#ifndef PT_SLEEP_H
#define PT_SLEEP_H


#include "pt/pt.h"
#include "timer0.h"


#define PT_SLEEP(pt, us)	do { _sleep=getCycleCount()+(us)*160; PT_WAIT_WHILE(pt, ((uint32_t)(getCycleCount() - _sleep)) & 0x8000000); } while(0)


#endif
