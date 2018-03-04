#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include "Thread.h"

#include "gpio_lib.h"
#include "tv.h"
#include "tv_data.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "vg75.h"
#include "kbd.h"
#include "timer0.h"
#include "ps2.h"


THREAD(main_thread)
{
    PT_BEGIN(pt);
	while (1)
	{
	    os_printf("Free heap size:%d\n", system_get_free_heap_size());
	    //os_printf("Hello world\n");
	    PT_SLEEP(200);
	}
    PT_END(pt);
}


extern "C" void user_init(void);
void ICACHE_FLASH_ATTR user_init()
{
    // Переводим проц на 160МГц
    system_update_cpu_freq(SYS_CPU_160MHZ);
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    os_delay_us(100000);
    
    os_printf("\n\n\n");
    os_printf("Free heap size: %d\n", system_get_free_heap_size());
    os_printf("SDK version:%s\n", system_get_sdk_version());
    
    // Входы-выходы
    os_printf("[GPIO]\n");
    gpio_init();
    
    // Нитки
    os_printf("[Threads]\n");
    threads_init();
    
    // i8080
    os_printf("[i8080]\n");
    i8080_hal_init();
    vg75_init();
    kbd_init();
    i8080_init();
    i8080_jump(0xF800);
    
    // Читаем игру
    spi_flash_read(0x80000, (uint32*)i8080_hal_memory(), 16384);
    
    
    ps2_init();
    while (1)
    {
	uint16_t code;
	
	ps2_periodic();
	code=ps2_read();
	if (code) os_printf("PS2: 0x%04X\n", code);
	switch (code)
	{
	    case PS2_0:
		ps2_leds(0,0,0);
		break;
	    
	    case PS2_1:
		ps2_leds(1,0,0);
		break;
	    
	    case PS2_2:
		ps2_leds(0,1,0);
		break;
	    
	    case PS2_3:
		ps2_leds(0,0,1);
		break;
	}
	
	os_delay_us(10);
	system_soft_wdt_feed();
    }
    
    
    // TV-out
    gpio_init_output(15);
    os_printf("[TV]\n");
    os_delay_us(200000);
    tv_init();
    
    
    uint32_t prev_T=getCycleCount();
    uint32_t cyc=0;
    uint32_t cnt=0;
    while (1)
    {
	uint8_t n=100;
	while (n--)
	    cyc+=i8080_instruction();
	
	uint32_t T=getCycleCount();
	uint32_t dT=T-prev_T;
	if (dT >= 160000000)
	{
	    // Прошла секунда
	    os_printf("Speed=%u\n", cyc);
	    cyc=0;
	    prev_T=T;
	    
	    cnt++;
	    if (cnt == 5)
	    {
		os_printf("Run game\n");
		i8080_jump(0x0000);
	    } else
	    if (cnt == 10)
	    {
		os_printf("Press enter\n");
		kbd_press(RK_VK);
	    }
	}
	
	tv_data_periodic();
	system_soft_wdt_feed();
    }
}
