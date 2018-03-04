#include "tv_data.h"

#include <osapi.h>
#include "tv.h"
#include "zkg.h"


#define N_BUFS	8


static uint8_t buf[N_BUFS][80], head=0, tail=0;

uint8_t screen_w=78, screen_h=30, char_h=8;
uint8_t cursor_x=0, cursor_y=0, cursor_type=0;
uint8_t *vram;

static inline void RAMFUNC empty_line(uint8_t *data)
{
    // Пустая линия
    uint8_t i, x=16;
    for (i=0; i<64; i++)
    {
	data[(x++) ^ 0x03]=0x00;
    }
}


static uint16_t line=0;
static uint8_t l=0, y=0, *txt;
static uint8_t blink=0;


static void RAMFUNC render_line(void)
{
    // Получаем буфер для рисования
    uint8_t *data=buf[tail];
    tail=(tail+1) & (N_BUFS-1);
    
    if ( (line < 12) || (line >= 304-12) )
    {
	// Пустые строки в начале и в конце кадра
	empty_line(data);
    } else
    {
	// Получаем номер линии в строке
	l++;
	if (l >= char_h)
	{
	    // Следующая строка текста
	    l=0;
	    txt+=screen_w;
	    y++;
	}
	
	// Проверим - видимая ли это строка
	if (l < 8)
	{
	    // Видимая линия
	    
	    // Получаем строку текста, всегда отображаем 78 символов
	    const uint8_t *z=zkg+( ((uint16_t)l) << 7);
	    uint8_t i, x=16;
	    uint8_t z1,z2,z3,z4;
	    uint8_t *t=txt;
	    
	    // Первые 3 символа (на экране у нас место для 80 символов, поэтому первый пропускаем)
	    z1=0;
	    z2=z[*t++];
	    z3=z[*t++];
	    z4=z[*t++];
	    data[(x++) ^ 0x03]=(z1 << 2) | (z2 >> 4);
	    data[(x++) ^ 0x03]=(z2 << 4) | (z3 >> 2);
	    data[(x++) ^ 0x03]=(z3 << 6) | z4;
	    
	    // Средние 72 символа
	    for (i=0; i<18; i++)
	    {
		z1=z[*t++];
		z2=z[*t++];
		z3=z[*t++];
		z4=z[*t++];
		data[(x++) ^ 0x03]=(z1 << 2) | (z2 >> 4);
		data[(x++) ^ 0x03]=(z2 << 4) | (z3 >> 2);
		data[(x++) ^ 0x03]=(z3 << 6) | z4;
	    }
	    
	    // Последние 3 символа
	    z1=z[*t++];
	    z2=z[*t++];
	    z3=z[*t++];
	    z4=0;
	    data[(x++) ^ 0x03]=(z1 << 2) | (z2 >> 4);
	    data[(x++) ^ 0x03]=(z2 << 4) | (z3 >> 2);
	    data[(x++) ^ 0x03]=(z3 << 6) | z4;
	    
	    // Рисуем курсор
	    if ( (y == cursor_y) && ( ((cursor_type & 0x01)==0) || (l==7) ) && ( (blink & 32) || ((cursor_type & 0x02)!=0) ) )
	    {
		uint8_t cx=cursor_x+1;	// т.к. у нас 80 символов, 1 символ получается пустой
		uint8_t p=16+(cx >> 2) * 3;	// позиция в буфере
		switch (cx & 0x03)
		{
		    case 0:
			data[(p+0) ^ 0x03]^=0xFC;
			break;
		    
		    case 1:
			data[(p+0) ^ 0x03]^=0x03;
			data[(p+1) ^ 0x03]^=0xF0;
			break;
		    
		    case 2:
			data[(p+1) ^ 0x03]^=0x0F;
			data[(p+2) ^ 0x03]^=0xC0;
			break;
		    
		    case 3:
			data[(p+2) ^ 0x03]^=0x3F;
			break;
		}
	    }
	} else
	{
	    // Пустая линия
	    empty_line(data);
	}
    }
    
    // Выбираем следующую строку
    line++;
    if (line==304)
    {
	line=0;
	
	// Начинаем с начала экрана
	l=0;
	y=0;
	txt=vram;
	
	// Мигание курсора
	blink++;
    }
}


void tv_data_init(void)
{
    uint8_t i;
    
    txt=vram;
    
    // Копируем в буфера пустую строку (в ней синхра) и заполняем их
    for (i=0; i<N_BUFS; i++)
    {
	os_memcpy(buf[i], tv_empty_line, 80);
	render_line();
    }
}


uint8_t* RAMFUNC tv_data_line(void)
{
    // Возвращаем следующий буфер в FIFO
    uint8_t *ret=buf[head];
    head=(head+1) & (N_BUFS-1);
    return ret;
}


void RAMFUNC tv_data_periodic(void)
{
    // Редерим строки до заполнения FIFO
    while (head != tail)
	render_line();
}
