#include "menu.h"

#include "ets.h"
#include "vg75.h"
#include "ps2.h"
#include "pt.h"
#include "xprintf.h"
#include "zkg.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "xlat.h"

#include "menu.h"


char ui_scr[38][78];


void ui_clear(void)
{
    ets_memset(ui_scr, 0x00, sizeof(ui_scr));
}


#define HEADER_Y	4
void ui_header(const char *s)
{
    static const uint8_t syms[]=
	{
	    0, 4, 16, 20, 2, 6, 18, 22,
	    1, 5, 17, 21, 3, 7, 19, 23
	};
    uint8_t l=ets_strlen(s);
    uint8_t x=(78-l*3)/2;	// каждый символ отображается как 3 символа
    uint8_t y;
    
    // Рисуем строки
    for (y=0; y<4; y++)
    {
	uint8_t i;
	for (i=0; i<l; i++)
	{
	    uint8_t c=xlat[(uint8_t)s[i]];
	    uint8_t b1=zkg[ ((y*2+0) << 7) + c];
	    uint8_t b2=zkg[ ((y*2+1) << 7) + c];
	    ui_scr[HEADER_Y+y][x+i*3+0]=syms[ ((b1 >> 2) & 0x0C) | ((b2 >> 4) & 0x03) ];
	    ui_scr[HEADER_Y+y][x+i*3+1]=syms[ (b1 & 0x0C) | ((b2 >> 2) & 0x03) ];
	    ui_scr[HEADER_Y+y][x+i*3+2]=syms[ ((b1 << 2) & 0x0C) | (b2  & 0x03) ];
	}
    }
}


#define LIST_Y	10
#define LIST_X	10
void ui_draw_list(const char *s)
{
    uint8_t y=LIST_Y;
    while (*s)
    {
	uint8_t x;
	for (x=LIST_X; (*s) && ((*s)!='\n'); x++)
	    ui_scr[y][x]=xlat[(uint8_t)(*s++)];
	if (*s) s++;	// пропускаем '\n'
	y++;
    }
}


int8_t ui_select_n=0, ui_select_count=0;
PT_THREAD(ui_select(struct pt *pt))
{
    static uint8_t prev=0;
    
    PT_BEGIN(pt);
	if (ui_select_n < 0) ui_select_n=0;
	while (1)
	{
	    // Очищаем предыдущую позицию
	    ets_memcpy(&ui_scr[LIST_Y+prev][LIST_X-4], "    ", 4);
	    
	    // Рисуем новую позицию
	    ets_memcpy(&ui_scr[LIST_Y+ui_select_n][LIST_X-4], "--> ", 4);
	    
	    // Сохраняем текущую позицию как предыдущую для перерисовки
	    prev=ui_select_n;
	    
	    // Читаем клаву
	    while (1)
	    {
		uint16_t code=ps2_read();
		if (code==PS2_ESC)
		{
		    // Отмена
		    ui_select_n=-1;
		    PT_EXIT(pt);
		} else
		if ( (code==PS2_ENTER) || (code==PS2_KP_ENTER) )
		{
		    // Ввод
		    PT_EXIT(pt);
		} else
		if ( (code==PS2_UP) && (ui_select_n > 0) )
		{
		    // Вверх
		    ui_select_n--;
		    break;
		} else
		if ( (code==PS2_DOWN) && (ui_select_n < ui_select_count-1) )
		{
		    // Вниз
		    ui_select_n++;
		    break;
		}
		
		// Ждем
		PT_YIELD(pt);
	    }
	}
    PT_END(pt);
}


void ui_start(void)
{
    // Сохраняем параметры экрана
    struct screen save=screen;
    
    // Перенастраиваем экран под себя
    screen.screen_w=78;
    screen.screen_h=38;
    screen.char_h=8;
    screen.cursor_x=0;
    screen.cursor_y=0;
    screen.cursor_type=0;
    screen.vram=(uint8_t*)ui_scr;
    
    // Очищаем экран
    ui_clear();
    
    // Инитим задачу меню
    struct pt pt;
    PT_INIT(&pt);
    
    // Цикл работы меню
    while (PT_SCHEDULE(menu(&pt)))
    {
	vg75_periodic();
	ps2_periodic();
    }
    
    
    // Возвращаем экран на место
    screen=save;
}
