#include "vg75.h"

#include "tv.h"
#include "zkg.h"
#include "i8080_hal.h"


struct screen screen;


static struct
{
    uint8_t cmd, param_n;
    uint8_t param[4];
} vg75;


static struct
{
    uint8_t param_n;
    uint8_t param[4];
} ik57;


#define N_BUFS	8
static uint8_t buf[N_BUFS][80];
static volatile uint8_t buf_n=0;


static inline void empty_line(uint8_t *data)
{
    // Пустая линия
    uint8_t i, x=16;
    for (i=0; i<64; i++)
    {
	data[(x++) ^ 0x03]=0x00;
    }
}


static uint16_t line=0;
static uint8_t l=0, y=0;
static uint8_t blink=0;
static uint8_t *txt;


static inline void render_line(uint8_t *data)
{
    if ( /*(line < 12) ||*/ (y >= screen.screen_h) )
    {
	// Пустые строки в начале и в конце кадра
	empty_line(data);
    } else
    {
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
	    if ( (y == screen.cursor_y) && ( ((screen.cursor_type & 0x01)==0) || (l==7) ) && ( (blink & 16) || ((screen.cursor_type & 0x02)!=0) ) )
	    {
		uint8_t cx=screen.cursor_x+1;	// т.к. у нас 80 символов, 1 символ получается пустой
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
	
	// Следующий номер линии в строке
	l++;
	if (l >= screen.char_h)
	{
	    // Следующая строка текста
	    l=0;
	    txt+=screen.screen_w;
	    y++;
	}
    }
    
    // Выбираем следующую строку
    line++;
}


void tv_data_field(void)
{
    // Начало поля
    line=0;
    l=0;
    y=0;
    txt=screen.vram;
    
    // Мигание курсора
    blink++;
}


uint8_t* tv_data_line(void)
{
    // Возвращаем следующий буфер в FIFO
    uint8_t *data=buf[(buf_n++) & (N_BUFS-1)];
    render_line(data);
    return data;
}


void vg75_init(uint8_t *vram)
{
    uint8_t i;
    
    screen.screen_w=78;
    screen.screen_h=30;
    screen.char_h=8;
    screen.cursor_x=0;
    screen.cursor_y=0;
    screen.cursor_type=0;
    screen.vram=vram;
    
    txt=vram;
    
    // Копируем в буфера пустую строку (в ней синхра)
    for (i=0; i<N_BUFS; i++)
	ets_memcpy(buf[i], tv_empty_line, 80);
}


void vg75_W(uint8_t A, uint8_t value)
{
    if (A)
    {
	// Команда
	vg75.cmd=value;
	vg75.param_n=0;
    } else
    {
	// Параметр
	vg75.param[vg75.param_n & 0x03]=value;
	vg75.param_n++;
	
	if ( (vg75.cmd==0x00) && (vg75.param_n==4) )
	{
	    // Сброс
	    screen.screen_w=(vg75.param[0] & 0x7F)+1;	// число символов в строке -1
	    screen.screen_h=(vg75.param[1] & 0x3F)+1;	// число строк на экране -1
	    screen.char_h=(vg75.param[2] & 0x0F)+1;	// высота символа в пикселах -1
	    screen.cursor_type=(vg75.param[3] >> 4) & 0x03;	// форма курсора: 0=мигающий блок, 1=мигающий штрих, 2=немигающий блок, 3=немигающий штрих
	    if (screen.screen_w > 78) screen.screen_w=78;
	    if (screen.screen_h > 38) screen.screen_h=38;
	    //ets_printf("VG75: W=%d H=%d CH=%d CUR=%d\n", screen.screen_w, screen.screen_h, screen.char_h, screen.cursor_type);
	} else
	if ( (vg75.cmd==0x80) && (vg75.param_n==2) )
	{
	    // Загрузить курсор
	    screen.cursor_x=vg75.param[0];
	    screen.cursor_y=vg75.param[1];
	    //ets_printf("VG75: cursor=%d,%d\n", screen.cursor_x, screen.cursor_y);
	}
    }
}


uint8_t vg75_R(uint8_t A)
{
    return 0xff;
}


void ik57_W(uint8_t A, uint8_t value)
{
    if ( (A==0x08) && (value==0x80) )
    {
	// Команда инициализации
	ik57.param_n=0;
    } else
    if ( (A==0x04) || (A==0x05) )
    {
	ik57.param[ik57.param_n++]=value;
	
	if (ik57.param_n==4)
	{
	    uint16_t vram_at=(ik57.param[1] << 8) | ik57.param[0];
	    //ets_printf("IK57: VRAM @%04x\n", vram_at);
	    screen.vram=i8080_hal_memory()+vram_at;
	    
	    ik57.param_n=0;
	}
    }
}


uint8_t ik57_R(uint8_t A)
{
    return 0x00;
}
