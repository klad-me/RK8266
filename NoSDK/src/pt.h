#ifndef PT_EXT_H
#define PT_EXT_H


#include "pt/pt.h"
#include "timer0.h"


#define PT_SLEEP(us)	do { _sleep=getCycleCount()+(us)*160; PT_WAIT_WHILE(pt, ((uint32_t)(getCycleCount() - _sleep)) & 0x80000000); } while(0)
#define PT_SUB(func)	do { PT_SPAWN(pt, &sub, func(&sub)); } while(0)


#endif
