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
#include "zkg.h"
#include "reboot.h"
#include "help.h"
#include "str.h"
#include "board.h"


// Отношение частоты ESP8266 к частоте эмуляции
volatile uint8_t i8080_speed_K=90;	// 160/1.78


void main_program(void)
{
    // Инитим файловую систему
    ffs_init();
    
    // Инитим процессор
    i8080_hal_init();
    i8080_init();
    i8080_jump(0xF800);
    
    // Читаем образы ПЗУ
    {
        int i;
	
        for (i=0; i<FAT_SIZE; i++)
        {
            if (fat[i].type == TYPE_ROM)
            {
                // Образ
                const char *name=ffs_name(i);
                ets_printf("Loading ROM '%s'\n", name);
                if ( (ets_strlen(name)!=4) ||
                     (! is_xdigit(name[0])) ||
                     (! is_xdigit(name[1])) ||
                     (! is_xdigit(name[2])) ||
                     (! is_xdigit(name[3])) )
                {
                    // Неверное имя файла
                    ets_printf("  Bad file name\n");
                    continue;
                }
		
                // Адрес
                int addr=parse_hex(name);
                if ( (addr >= 0xE000) && (addr+fat[i].size <= 0x10000) )
                {
                    // ПЗУ
                    // Загружаем в IRAM (можно использовать ffs_read, т.к. он работает с 4-байтными словами)
                    ffs_read(i, 0, i8080_hal_rom()+(addr-0xE000), fat[i].size);
                    ets_printf("  OK\n");
                } else
                if ( (addr < 0x8000) && (addr+fat[i].size <= 0x8000) )
                {
                    // ОЗУ
                    ffs_read(i, 0, i8080_hal_memory()+addr, fat[i].size);
                    ets_printf("  OK\n");
                } else
                {
                    // Неверный адрес или размер
                    ets_printf("  Bad address or size\n");
                    continue;
                }

            }
        }
    }
    
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
        	prev_T+=cycles*i8080_speed_K; else
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
	    uint16_t c;
	    bool rst=false;
	    
    	    ps2_leds(kbd_rus(), true, turbo);
    	    ps2_periodic();
    	    c=keymap_periodic();
    	    switch (c)
    	    {
    		case 0:
    		    break;
    		
    		case PS2_ESC:
    		    // Меню
		    ui_start();
			menu();
		    ui_stop();
		    rst=true;
		    break;
		
		case PS2_F5:
		    // Переход на ПЗУ
		    i8080_jump(0xE000);
		    break;
		
		case PS2_F6:
		    // Переход на ПЗУ
		    i8080_jump(0xE004);
		    break;
		
		case PS2_F7:
		    // Переход на ПЗУ
		    i8080_jump(0xE008);
		    break;
		
		case PS2_F8:
		    // Переход на ПЗУ
		    i8080_jump(0xE00C);
		    break;
		
		case PS2_F10:
		    // Переход на ПЗУ
		    i8080_jump(0xE010);
		    break;
		
		case PS2_F9:
		    // Переход на ПЗУ
		    i8080_jump(0xE014);
		    break;
		
		case PS2_F11:
		    // Выход в монитор
		    i8080_jump(0xF800);
		    break;
		
		case PS2_F12:
		    // Файловый менеджен
		    ui_start();
			menu_fileman();
		    ui_stop();
		    rst=true;
		    break;
		
		case PS2_PAUSE:
		    // Сброс
        	    i8080_init();
        	    i8080_hal_init();
        	    i8080_jump(0xF800);
        	    break;
		
		case PS2_PRINT:
		    // WiFi
		    reboot(0x55AA55AA);
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
		
		case PS2_MENU:
		    // Отобразить справку
		    help_display();
		    break;
		
		/*case PS2_F12:
		    // Дамп экрана
		    {
			ets_printf("VRAM w=%d h=%d:\n", screen.screen_w, screen.screen_h);
			int i,j;
			uint8_t *vram=screen.vram;
			for (i=0; i<screen.screen_h; i++)
			{
			    ets_printf("%2d: ", i);
			    for (j=0; j<screen.screen_w; j++)
			    {
				ets_printf(" %02X", *vram++);
			    }
			    ets_printf("\n");
			}
		    }
		    break;*/
		
		default:
		    ets_printf("PS2: %04X\n", c);
		    break;
    	    }
    	    
    	    if (rst)
    	    {
	        // Сбрасываем время циклов
		sec_T=prev_T=getCycleCount();
		sec_cycles=0;
	    }
    	}
    }
}
