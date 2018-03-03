#ifndef LIBESP_THREAD_H
#define LIBESP_THREAD_H


#include <c_types.h>
#include <osapi.h>
#include "pt/pt.h"


#ifdef __cplusplus

class Thread
{
public:
    virtual void start();
    virtual PT_THREAD(run(struct pt *pt)) = 0;
    
    
    struct pt _thread;
    os_timer_t _thread_tmr;
};

#endif


typedef char (*c_thread_cb)(struct pt *pt);


#ifdef __cplusplus
extern "C" {
#endif


extern uint16_t thread_sleep;


void threads_init(void);
void c_thread_start(c_thread_cb cb);


#define THREAD(NAME) \
    static PT_THREAD(task_##NAME(struct pt *pt));       \
    void NAME(void)     \
    {   \
        c_thread_start(task_##NAME); \
    }   \
    static PT_THREAD(task_##NAME(struct pt *pt))


#define PT_SLEEP(ms)    do { thread_sleep=(ms); PT_YIELD(pt); } while(0)
#define PT_SUB(name)    do { PT_SPAWN(pt, &sub, name(&sub)); } while(0)


#ifdef __cplusplus
}
#endif


#endif
