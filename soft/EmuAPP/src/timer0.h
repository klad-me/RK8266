#ifndef TIMER0_H
#define TIMER0_H


#include "ets.h"


#ifdef __cplusplus
extern "C" {
#endif


#define ETS_CCOMPARE0_INUM  6



typedef void(*timercallback)(void);


extern volatile timercallback timer0_user_cb;


static inline uint32_t ETS_INTR_PENDING(void)
{
    uint32_t pending;
    __asm__ __volatile__("esync; rsr %0,interrupt":"=a" (pending));
    return pending;
}


static inline uint32_t getCycleCount()
{
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount":"=a" (ccount));
    return ccount;
}



typedef void(*timercallback)(void);


// timer0 is a special CPU timer that has very high resolution but with
// limited control.
// it uses CCOUNT (ESP.GetCycleCount()) as the non-resetable timer counter
// it does not support divide, type, or reload flags
// it is auto-disabled when the compare value matches CCOUNT
// it is auto-enabled when the compare value changes
#define timer0_interrupted()    (ETS_INTR_PENDING() & (_BV(ETS_CCOMPARE0_INUM)))
#define timer0_read() ((__extension__({uint32_t count;__asm__ __volatile__("esync; rsr %0,ccompare0":"=a" (count));count;})))
#define timer0_write(count) __asm__ __volatile__("wsr %0,ccompare0; esync"::"a" (count) : "memory")

void timer0_isr_init(void);
void timer0_attachInterrupt(timercallback userFunc);
void timer0_detachInterrupt(void);


#ifdef __cplusplus
}
#endif


#endif
