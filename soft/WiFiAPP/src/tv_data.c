#include "tv_data.h"

#include <osapi.h>
#include "tv.h"
#include "font_8x12.h"


#define N_BUFS	8	// должно быть больше 4

static uint8_t buf[N_BUFS][80];
static volatile uint8_t buf_n=0;

uint8_t cursor_x=0, cursor_y=0;
char vram[20][50];


static uint16_t line=0;
static uint8_t l=0, y=0, *txt=(uint8_t*)vram;
static uint8_t blink=0;


static void RAMFUNC render_line(uint8_t *data)
{
    if ( (line < 38) || (y>=20) )
    {
	// Пустые строки в начале и в конце кадра
	uint8_t i, x=21;
	for (i=0; i<50; i++)
	{
	    data[(x++) ^ 0x03]=0x00;
	}
    } else
    {
	// Рисуем строку
	const uint8_t *z=font_8x12+( ((uint16_t)l) << 8 );
	uint8_t i, x=21;
	uint8_t *t=txt;
	if (y==0)
	{
	    // Строка 0 - выделяем ее (это статус)
	    for (i=0; i<50; i++)
		data[(x++) ^ 0x03]=z[*t++] ^ 0xff;
	} else
	{
	    for (i=0; i<50; i++)
		data[(x++) ^ 0x03]=z[*t++];
	}
	
	// Рисуем курсор
	if ( (y == cursor_y) && (cursor_x < 50) && (blink & 16) )
	{
	    data[(21+cursor_x) ^ 0x03]^=0xFF;
	}
	
	// Следующий номер линии в строке
	l++;
	if (l >= 12)
	{
	    // Следующая строка текста
	    l=0;
	    txt+=50;
	    y++;
	}
    }
    
    // Выбираем следующую строку
    line++;
}


void tv_data_init(void)
{
    uint16_t i;
    
    memset(vram, 0x00, sizeof(vram));
    
    // Копируем в буфера пустую строку (в ней синхра)
    for (i=0; i<N_BUFS; i++)
	os_memcpy(buf[i], tv_empty_line, 80);
}


void RAMFUNC tv_data_field(void)
{
    // Начало поля
    line=0;
    l=0;
    y=0;
    txt=(uint8_t*)vram;
    
    // Мигание курсора
    blink++;
}


uint8_t* RAMFUNC tv_data_line(void)
{
    // Возвращаем следующий буфер в FIFO
    uint8_t *data=buf[(buf_n++) & (N_BUFS-1)];
    render_line(data);
    return data;
}
