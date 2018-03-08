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


uint32_t flash_size(void)
{
    uint32_t size = 512*1024;
    uint32_t x1[8], x2[8];
    if (SPIRead(0, x1, 8*4) == 0)
    {
        for ( ; size<16384*1024; size<<=1)
        {
            if ( (SPIRead(size, x2, 8*4) != 0) ||
        	 (ets_memcmp(x1, x2, 8*4) == 0) )
    		break;
        }
    }
    return size;
}


void main_program(void)
{
    // Инитим процессор
    i8080_hal_init();
    i8080_init();
    i8080_jump(0xF800);
    
    ets_printf("Flash size=%d\n", flash_size());

    // Читаем игру
    SPIRead(0x80000, (uint32_t*)i8080_hal_memory(), 6661);
    
    
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
            uint8_t n=100;
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
            ets_printf("Speed=%d reg=0x%08x\n", (int)sec_cycles, READ_PERI_REG(0x3ff00024));
            //kbd_dump();
            sec_cycles=0;
            sec_T=T;
        }
	
        vg75_periodic();
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
