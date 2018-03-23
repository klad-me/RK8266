#include "ui.h"

#include "ets.h"
#include "vg75.h"
#include "ps2.h"
#include "ps2_codes.h"
#include "xprintf.h"
#include "zkg.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "xlat.h"
#include "align4.h"
#include "ffs.h"
#include "timer0.h"

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
	    uint8_t c=r_u8(&xlat[(uint8_t)s[i]]);
	    uint8_t b1=r_u8(&zkg[ ((y*2+0) << 7) + c]);
	    uint8_t b2=r_u8(&zkg[ ((y*2+1) << 7) + c]);
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
	    ui_scr[y][x]=r_u8(&xlat[(uint8_t)(*s++)]);
	if (*s) s++;	// пропускаем '\n'
	y++;
    }
}


void ui_draw_text(uint8_t x, uint8_t y, const char *s)
{
    while (*s)
    {
	uint8_t xx;
	for (xx=x; (*s) && ((*s)!='\n'); xx++)
	    ui_scr[y][xx]=r_u8(&xlat[(uint8_t)(*s++)]);
	if (*s) s++;	// пропускаем '\n'
	y++;
    }
}


int8_t ui_select(uint8_t count)
{
    uint8_t n=0, prev=0;
    
    while (1)
    {
	// Очищаем предыдущую позицию
	ets_memcpy(&ui_scr[LIST_Y + (prev % 20)][LIST_X-4 + (prev / 20)*16], "    ", 4);
	
	// Рисуем новую позицию
	ets_memcpy(&ui_scr[LIST_Y + (n % 20)][LIST_X-4 + (n / 20)*16], "--> ", 4);
	
	// Сохраняем текущую позицию как предыдущую для перерисовки
	prev=n;
	
	// Читаем клаву
	while (1)
	{
	    char c=ps2_sym();
	    if (c==KEY_ESC)
	    {
		// Отмена
		return -1;
	    } else
	    if (c==KEY_ENTER)
	    {
	        // Ввод
	        return n;
	    } else
	    if ( (c==KEY_UP) && (n > 0) )
	    {
		// Вверх
		n--;
		break;
	    } else
	    if ( (c==KEY_DOWN) && (n < count-1) )
	    {
		// Вниз
		n++;
		break;
	    } else
	    if ( (c==KEY_LEFT) && (n > 0) )
	    {
		// Влево
		n-=20;
		if (n < 0) n=0;
		break;
	    } else
	    if ( (c==KEY_RIGHT) && (n < count-1) )
	    {
		// Вправо
		n+=20;
		if (n >= count) n=count-1;
		break;
	    } else
	    if ( (c>='1') && (c<='9') )
	    {
		// Выбор нужного пункта по номеру
		if (c-'1' < count)
		{
		    return c-'1';
		}
	    }
	}
    }
}


int16_t ui_select_file(uint8_t type)
{
#define MAX_FILES	80
    uint8_t filelist[MAX_FILES];
    uint8_t n_files;
    uint16_t i;
    
    // Собираем каталог файлов
    n_files=0;
    for (i=0; i<FAT_SIZE; i++)
    {
	if (fat[i].type == type)
	{
	    filelist[n_files++]=i;
	    if (n_files>=MAX_FILES) break;
	}
    }
    
    // Рисуем
    ui_clear();
    ui_header("РАДИО-86РК -->");
    if (n_files==0)
    {
	ui_draw_text(10, 10, "Нет файлов !");
	ui_sleep(1000);
	return -1;
    }
    
    ui_draw_text(10, 8, "Выберите файл для загрузки:");
    for (i=0; i<n_files; i++)
    {
	ui_draw_text(10+(i/20)*16, 10+(i%20), ffs_name(filelist[i]));
    }
    int8_t n=ui_select(n_files);
    if (n >= 0)
	return filelist[n]; else
	return -1;
}


const char* ui_input_text(const char *comment, uint8_t max_len)
{
    static char text[64];
    uint8_t pos;
    char c;
    
#define EDIT_X	10
#define EDIT_Y	9
    pos=0;
    screen.cursor_x=EDIT_X;
    screen.cursor_y=EDIT_Y;
    ui_clear();
    ui_header("РАДИО-86РК -->");
    ui_draw_text(10, 8, comment);
    while (1)
    {
	c=ps2_sym();
	if (c==KEY_ESC)
	{
	    // Отмена
	    return 0;
	} else
	if ( (c==KEY_ENTER) && (pos > 0) )
	{
	    // Сохранение
	    text[pos]=0;
	    return text;
	} else
	if ( (c==KEY_BACKSPACE) && (pos > 0) )
	{
	    // Забой
	    screen.cursor_x--;
	    pos--;
	    ui_scr[EDIT_Y][EDIT_X+pos]=' ';
	} else
	if ( (c>=32) && (c<128) && (pos < max_len) )
	{
	    // Символ
	    text[pos]=c;
	    ui_scr[EDIT_Y][EDIT_X+pos]=c;
	    screen.cursor_x++;
	    pos++;
	}
    }
}



static struct screen save;


void ui_start(void)
{
    // Сохраняем параметры экрана
    save=screen;
    
    // Перенастраиваем экран под себя
    screen.screen_w=78;
    screen.screen_h=38;
    screen.char_h=8;
    screen.cursor_x=0;
    screen.cursor_y=0;
    screen.cursor_type=0;
    screen.vram=(uint8_t*)ui_scr;
    screen.overlay_timer=0;
    
    // Очищаем экран
    ui_clear();
}


void ui_stop(void)
{
    // Возвращаем экран на место
    save.overlay_timer=0;
    screen=save;
}


void ui_sleep(uint16_t ms)
{
    uint32_t _sleep=getCycleCount()+(ms)*160000;
    while (((uint32_t)(getCycleCount() - _sleep)) & 0x80000000);
}
