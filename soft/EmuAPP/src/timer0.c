#include "timer0.h"


#define ETS_CCOMPARE0_INTR_ATTACH(func, arg) \
    ets_isr_attach(ETS_CCOMPARE0_INUM, func, (void *)(arg))

#define ETS_CCOMPARE0_ENABLE() \
    ETS_INTR_ENABLE(ETS_CCOMPARE0_INUM)

#define ETS_CCOMPARE0_DISABLE() \
    ETS_INTR_DISABLE(ETS_CCOMPARE0_INUM)


volatile timercallback timer0_user_cb = NULL;


void timer0_isr_handler(void* para)
{
    if (timer0_user_cb)
    {
        // to make ISR compatible to Arduino AVR model where interrupts are disabled
        // we disable them before we call the client ISR
        //uint32_t savedPS = xt_rsil(15); // stop other interrupts
        timer0_user_cb();
        //xt_wsr_ps(savedPS);
    }
}


void timer0_isr_init()
{
    ETS_CCOMPARE0_INTR_ATTACH(timer0_isr_handler, NULL);
}


void timer0_attachInterrupt(timercallback userFunc)
{
    timer0_user_cb = userFunc;
    ETS_CCOMPARE0_ENABLE();
}

void timer0_detachInterrupt()
{
    timer0_user_cb = NULL;
    ETS_CCOMPARE0_DISABLE();
}
