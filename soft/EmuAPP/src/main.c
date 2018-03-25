#include "ets.h"

#include "tv.h"
#include "vg75.h"
#include "vv55_i.h"
#include "ps2.h"
#include "keymap.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "tape.h"
#include "timer0.h"
#include "ui.h"
#include "menu.h"
#include "ffs.h"
#include "board.h"


void main_program(void)
{
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
    
    // Инитим магнитофон
    tape_init();
    
    // Запускаем эмуляцию
    uint32_t prev_T=getCycleCount();
    uint32_t sec_T=prev_T;
    uint32_t cycles=0, sec_cycles=0;
    bool turbo=false, win=false;
    while (1)
    {
        uint32_t T=getCycleCount();
        int32_t dT=T-prev_T;
	
        if ( (dT > 0) || (turbo) )
        {
            // Можно запускать эмуляцию проца
            uint8_t n=turbo ? 200 : 20;
            while (n--)
            {
        	uint16_t c=i8080_instruction();
                cycles+=c;
                i8080_cycles+=c;
            }
	    
            if (! turbo)
        	prev_T+=cycles*90; else
        	prev_T=T;
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
	
	// Вся периодика
	
	if (tape_periodic())
	{
	    // Закончена запись на магнитофон - надо предложить сохранить файл
	    ui_start();
		tape_save();
	    ui_stop();
	    
	    // Сбрасываем время циклов
	    sec_T=prev_T=getCycleCount();
	    sec_cycles=0;
	}
	
	if (win)
	{
	    // Win нажата - обрабатываем спец-команды
	    uint16_t c=ps2_read();
	    switch (c)
	    {
		case PS2_LEFT:
		    // Экран влево
		    if (screen.x_offset > 0) screen.x_offset--;
		    break;
		    
		case PS2_RIGHT:
		    // Экран вправо
		    if (screen.x_offset < 16) screen.x_offset++;
		    break;
		    
		case PS2_UP:
		    // Экран вверх
		    if (screen.y_offset > 8) screen.y_offset-=8; else screen.y_offset=0;
		    break;
		    
		case PS2_DOWN:
		    // Экран вниз
		    if (screen.y_offset < 8*8) screen.y_offset+=8;
		    break;
		    
		case PS2_L_WIN | 0x8000:
		case PS2_R_WIN | 0x8000:
		    // Отжали Win
		    win=false;
		    break;
	    }
	} else
	{
	    // Win не нажата
    	    ps2_leds(kbd_rus(), false, turbo);
    	    ps2_periodic();
    	    switch (keymap_periodic())
    	    {
    		case PS2_ESC:
    		    // Нажали ESC - запуск меню
		    ui_start();
			menu();
		    ui_stop();
		    
		    // Сбрасываем время циклов
		    sec_T=prev_T=getCycleCount();
		    sec_cycles=0;
		    break;
		
		case PS2_SCROLL:
		    // Переключатель турбо
		    turbo=!turbo;
		    break;
		
		case PS2_L_WIN:
		case PS2_R_WIN:
		    // Нажали Win
		    win=true;
		    break;
    	    }
    	}
    }
}
