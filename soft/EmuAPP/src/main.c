#include "ets.h"

#include "tv.h"
#include "vg75.h"
#include "vv55_i.h"
#include "ps2.h"
#include "keymap.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "timer0.h"
#include "ui.h"
#include "ffs.h"
#include "board.h"


static void my_putc1(char c)
{
}


void main_program(void)
{
#ifndef DO_DEBUG
    // Заменяем ets_printf на себя
    ets_install_putc1((void*)my_putc1);
#endif
    
    // Инитим файловую систему
    ffs_init();
    
    // Инитим процессор
    i8080_hal_init();
    i8080_init();
    i8080_jump(0xF800);
    
    // Инитим экран
    tv_init();
    vg75_init((uint8_t*)i8080_hal_memory());
    tv_start();
    
    // Инитим клавиатуру
    kbd_init();
    ps2_init();
    keymap_init();
    
    // Запускаем эмуляцию
    uint32_t prev_T=getCycleCount();
    uint32_t sec_T=prev_T;
    uint32_t cycles=0, sec_cycles=0;
    while (1)
    {
        uint32_t T=getCycleCount();
        int32_t dT=T-prev_T;
	
        if (dT > 0)
        {
            // Можно запускать эмуляцию проца
            uint8_t n=20;
            while (n--)
            {
        	uint16_t c=i8080_instruction();
                cycles+=c;
                i8080_cycles+=c;
            }
	    
            prev_T+=cycles*90;
            sec_cycles+=cycles;
            cycles=0;
        }
	
        if ( ((uint32_t)(T-sec_T)) >= 160000000)
        {
            // Прошла секунда
            ets_printf("Speed=%d rtc=0x%08x\n", (int)sec_cycles, READ_PERI_REG(0x60001200));
            //kbd_dump();
            sec_cycles=0;
            sec_T=T;
        }
	
        ps2_leds(kbd_rus(), kbd_rus(), kbd_rus());
        ps2_periodic();
        if (keymap_periodic())
        {
    	    // Нажали ESC - запуск меню
	    ui_start();
	    
	    // Сбрасываем время циклов
	    sec_T=prev_T=getCycleCount();
	    sec_cycles=0;
        }
    }
}
