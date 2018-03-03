#include "Thread.h"

#include <user_interface.h>


class C_Thread : public Thread
{
public:
    C_Thread(c_thread_cb _cb)
    {
	cb=_cb;
	start();
    }
    
    PT_THREAD(run(struct pt *pt))
    {
	return cb(pt);
    }
    
private:
    c_thread_cb cb;
};

#define THREAD_PRIO     0


static os_event_t thread_q[16];
uint16_t thread_sleep;


static void thread_sched(Thread *thr)
{
    thread_sleep=0;
    if (PT_SCHEDULE(thr->run(&thr->_thread)))
    {
        // Еще работает задача
        if (thread_sleep > 0)
        {
            // Уснуть на время
            os_timer_disarm(&thr->_thread_tmr);
            os_timer_arm(&thr->_thread_tmr, thread_sleep, 0);
        } else
        {
            // Переключить контекст
            system_os_post(THREAD_PRIO, 0, (os_param_t)thr);
        }
    }
}


static void thread_task(os_event_t *evt)
{
    thread_sched((Thread*)evt->par);
}


void threads_init(void)
{
    system_os_task(thread_task, THREAD_PRIO, thread_q, sizeof(thread_q) / sizeof(os_event_t));
}


void Thread::start()
{
    PT_INIT(&_thread);
    os_timer_setfn(&_thread_tmr, (os_timer_func_t*)thread_sched, (void*)this);
    system_os_post(THREAD_PRIO, 0, (os_param_t)this);
}


void c_thread_start(c_thread_cb cb)
{
    new C_Thread(cb);
}
