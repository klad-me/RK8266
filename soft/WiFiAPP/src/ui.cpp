#include "ui.h"

#include <osapi.h>
#include "tv_data.h"
#include "ps2.h"
#include "ps2_codes.h"
#include "Thread.h"


void ui_clear(void)
{
    // Очищаем все, кроме строки статуса
    os_memset(&vram[1][0], 0x00, sizeof(vram)-50);
}


void ui_header(const char *s)
{
    strcpy(&vram[2][0], s);
}


#define LIST_Y	4
#define LIST_X	4
void ui_draw_list(const char *s)
{
    uint8_t y=LIST_Y;
    while (*s)
    {
	for (uint8_t x=LIST_X; (*s) && ((*s)!='\n'); x++)
	    vram[y][x]=(*s++);
	if (*s) s++;	// пропускаем '\n'
	y++;
    }
}


void ui_draw_text(uint8_t x, uint8_t y, const char *s)
{
    while (*s)
    {
	for (uint8_t xx=x; (*s) && ((*s)!='\n'); xx++)
	    vram[y][xx]=(*s++);
	if (*s) s++;	// пропускаем '\n'
	y++;
    }
}


int8_t ui_select_n=0, ui_select_count=0;
PT_THREAD(ui_select(struct pt *pt))
{
    static uint8_t prev=0;
    char c;
    
    PT_BEGIN(pt);
	if (ui_select_n < 0) ui_select_n=0;
	while (1)
	{
	    // Очищаем предыдущую позицию
	    os_memcpy(&vram[LIST_Y+prev][LIST_X-4], "    ", 4);
	    
	    // Рисуем новую позицию
	    os_memcpy(&vram[LIST_Y+ui_select_n][LIST_X-4], "--> ", 4);
	    
	    // Сохраняем текущую позицию как предыдущую для перерисовки
	    prev=ui_select_n;
	    
	    // Читаем клаву
	    while (1)
	    {
		c=ps2_sym();
		if (c==KEY_ESC)
		{
		    // Отмена
		    ui_select_n=-1;
		    PT_EXIT(pt);
		} else
		if (c==KEY_ENTER)
		{
		    // Ввод
		    PT_EXIT(pt);
		} else
		if ( (c==KEY_UP) && (ui_select_n > 0) )
		{
		    // Вверх
		    ui_select_n--;
		    break;
		} else
		if ( (c==KEY_DOWN) && (ui_select_n < ui_select_count-1) )
		{
		    // Вниз
		    ui_select_n++;
		    break;
		} else
		if ( (c>='1') && (c<='9') )
		{
		    // Выбор нужного пункта по номеру
		    uint8_t n=c-'1';
		    if (n < ui_select_count)
		    {
			ui_select_n=n;
			PT_EXIT(pt);
		    }
		}
		
		// Ждем
		PT_SLEEP(10);
	    }
	}
    PT_END(pt);
}
