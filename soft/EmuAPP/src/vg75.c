#include "vg75.h"

#include "tv.h"
#include "zkg.h"
#include "i8080_hal.h"
#include "align4.h"
#include "xlat.h"


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


// Графические буфера (I2S работает с задержкой, потому нельзя использовать 2 буфера - надо больше)
#define N_BUFS	8
static uint8_t buf[N_BUFS][64];
static volatile uint8_t buf_n=0;


#define empty_line(data)	do { ets_memset((data)+4, 0x00, 60); } while(0)


// Аттрибуты и текст
#define HIGHLIGHT	0x01
#define BLINK		0x02
#define GPA0		0x04
#define GPA1		0x08
#define REVERSE		0x10
#define UNDERLINE	0x20

static uint16_t line=0;
static uint8_t l=0, y=0;
static uint8_t blink=0, flags=0;
static uint8_t *txt;
static uint8_t line_text[120], line_attr[120];	// 120 для возможности сдвига изображения вправо
static uint8_t end_of_screen=0;


// Регистр статуса
#define SREG_IE	0x40
#define SREG_IR	0x20
#define SREG_LP	0x10
#define SREG_IC	0x08
#define SREG_VE	0x04
#define SREG_DU	0x02
#define SREG_FO	0x01

static uint8_t sreg=0x00;
static uint8_t was_IR=0;


// Псевдографика
#define PG_LEFT		0x01
#define PG_RIGHT	0x02
#define PG_TOP		0x04
#define PG_BOTTOM	0x08
#define PG_VERT		(PG_TOP | PG_BOTTOM)
#define PG_HORIZ	(PG_LEFT | PG_RIGHT)

static const uint8_t pg_tab[16]=
{
    PG_BOTTOM	| PG_RIGHT,	// 0
    PG_BOTTOM	| PG_LEFT,	// 1
    PG_TOP	| PG_RIGHT,	// 2
    PG_TOP	| PG_LEFT,	// 3
    PG_BOTTOM	| PG_HORIZ,	// 4
    PG_VERT	| PG_LEFT,	// 5
    PG_VERT	| PG_RIGHT,	// 6
    PG_TOP	| PG_HORIZ,	// 7
		  PG_HORIZ,	// 8
    PG_VERT,			// 9
    PG_VERT	| PG_HORIZ,	// A
};



#define OVERLAY_Y	3


static void next_line(void)
{
    uint8_t x=0, o=0;
    uint8_t l=screen.screen_w;	// кол-во символов в строке
    
    // Если конец экрана - дальше не обрабатываем
    if (end_of_screen) goto line_done;
    
    // Отрабатываем сдвиг экрана
    while (x < screen.x_offset)
    {
	line_text[x]=0;
	line_attr[x]=flags;
	x++;
    }
    
    while (l--)
    {
	// Получаем следующий символ
	uint8_t c=txt[o++];
	
	// Проверяем на спец.коды
	if (! (c & 0x80))
	{
	    // Обычный символ из знакогенератора
	    line_text[x]=( (!(flags & BLINK)) || (blink & 16) ) ? c : 0;
	    line_attr[x]=flags;
	    x++;
	} else
	{
	    // Управляющий код, псевдографика или аттрибут
	    if (c & 0x40)
	    {
		// Управлящий код или псевдографика
		if ( (c & 0xFC) == 0xF0 )
		{
		    // Управляющий код
		    switch (c & 0x03)
		    {
			case 0:
			    // End of Row
			    o=screen.screen_w;
			    goto line_done;
			
			case 1:
			    // End of Row - Stop DMA
			    if ( ((o & (screen.dma_burst-1))==0) ||
				 (o==screen.screen_w) )
			    {
				// Конец пачки DMA или конец строки - добавлять символ не надо
				goto line_done;
			    } else
			    {
				// Надо пропустить один символ
				o++;
				goto line_done;
			    }
			
			case 2:
			case 3:
			    // End of Screen
			    // End of Screen - Stop DMA
			    end_of_screen=1;
			    goto line_done;
		    }
		} else
		{
		    // Псевдографика
		    line_text[x]=( (!(c & BLINK)) || (blink & 16) ) ? (((c >> 2) & 0x0F) | 0x80) : 0;
		    line_attr[x]=(flags & 0xFC) | (c & 0x03);	// используем общие аттрибуты, но заменяем в них B и H
		    x++;
		}
	    } else
	    {
		// Аттрибут
		flags=c & 0x3F;
		if (screen.attr_visible)
		{
		    // Аттрибут видимый - отображаем как символ 0x00
		    line_text[x]=0x00;
		    line_attr[x]=flags;
		    x++;
		}
	    }
	}
    }
    
line_done:
    // Дозаполняем остаток строки
    while (x < 80)
    {
	line_text[x]=0;
	line_attr[x]=flags;
	x++;
    }
    
    // Сдвигаем указатель на видеопамять
    txt+=o;
    
    // Рисуем курсор
    if ( (y == screen.cursor_y) && (screen.cursor_x < 80) && ( (blink & 16) || ((screen.cursor_type & 0x02)!=0) ) )
    {
	// Видимый
	if (screen.cursor_type & 0x01)
	{
	    // Штрих
	    line_attr[screen.x_offset+screen.cursor_x]^=UNDERLINE;
	} else
	{
	    // Блок
	    line_attr[screen.x_offset+screen.cursor_x]^=REVERSE;
	}
    }
    
    
    // Если оверлей активен - рисуем его
    if ( (screen.overlay_timer > 0) &&
	 (y == OVERLAY_Y) )
    {
	ets_memcpy(line_text+screen.x_offset+8, screen.overlay, 64);
	ets_memset(line_attr+screen.x_offset+8, REVERSE, 64);
    }
    
    // Пустые символы в начале и конце строки, чтобы не портить синхронизацию
    line_text[0]=line_text[1]=line_text[78]=line_text[79]=0;
    line_attr[0]=line_attr[1]=line_attr[78]=line_attr[79]=0;
    
    
    // Если конец экрана или последняя строка - то ставим флаг прерывания
    if ( (end_of_screen) || (y==screen.screen_h-1) )
    {
	if (! was_IR)
	{
	    sreg|=SREG_IR;
	    was_IR=1;
	}
    }
}


static inline void render_line(uint8_t *data)
{
    if ( (line < screen.y_offset) || (y >= screen.screen_h) )
    {
	// Пустые строки в начале и в конце кадра
	empty_line(data);
    } else
    {
	// Видимая линия
	const uint8_t *z=zkg+( ((uint16_t)l) << 7);
	uint8_t i, x=4, o=0;
	uint8_t z1,z2,z3,z4;
	
	// Рисуем 80 символов из подготовленного буфера (пустые символы в начале и в конце уже лежат в буфере)
	for (i=0; i<20; i++)
	{
	    uint8_t c, a;
	    
#define SYM(zz) \
	    do { \
		c=line_text[o];	\
		a=line_attr[o];	\
		o++;	\
		if (c & 0x80)	\
		{	\
		    c=pg_tab[c & 0x0f];	\
		    if (l < screen.underline_y)	\
		    {	\
			zz=(c & PG_TOP) ? 0x08 : 0x00;	\
		    } else	\
		    if (l > screen.underline_y)	\
		    {	\
			zz=(c & PG_BOTTOM) ? 0x08 : 0x00;	\
		    } else	\
		    {	\
			zz=((c & PG_LEFT) ? 0x38 : 0x00) | ((c & PG_RIGHT) ? 0x0F : 0x00) | ((c & PG_VERT) ? 0x08 : 0x00);	\
		    }	\
		} else	\
		{	\
		    zz=(l & 0x08) ? 0x00 : z[c];	\
		}	\
		if ( (a & UNDERLINE) && (l==screen.underline_y) ) zz|=0x3F;	\
		if (a & REVERSE) zz^=0x3F;	\
	    } while(0)
	    
	    SYM(z1);
	    SYM(z2);
	    SYM(z3);
	    SYM(z4);
	    data[(x++) ^ 0x03]=(z1 << 2) | (z2 >> 4);
	    data[(x++) ^ 0x03]=(z2 << 4) | (z3 >> 2);
	    data[(x++) ^ 0x03]=(z3 << 6) | z4;
	}
	
	// Следующий номер линии в строке
	l++;
	if (l >= screen.char_h)
	{
	    // Следующая строка текста
	    l=0;
	    y++;
	    next_line();
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
    
    // Таймер оверлея
    if (screen.overlay_timer > 0)
	screen.overlay_timer--;
    
    // Флаги
    end_of_screen=0;
    flags=0;
    was_IR=0;
    
    // Получаем первую строку
    next_line();
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
    screen.underline_y=7;
    screen.char_h=8;
    screen.attr_visible=0;
    screen.x_offset=4;
    screen.y_offset=8;
    screen.cursor_x=0;
    screen.cursor_y=0;
    screen.cursor_type=0;
    screen.dma_burst=1;
    screen.vram=vram;
    screen.overlay_timer=0;
    
    txt=vram;
    
    // Копируем в буфера пустую строку (в ней синхра)
    for (i=0; i<N_BUFS; i++)
	ets_memcpy(buf[i], tv_empty_line, 64);
    
    // Инитим знакогенератор в ОЗУ
    ets_memcpy(zkg, zkg_rom, 1024);
}


void vg75_overlay(const char *str)
{
    uint8_t l=ets_strlen(str);
    uint8_t p=0;
    
    // Пустое место слева
    while (p < (64-l)/2)
	screen.overlay[p++]=0;
    
    // Сама строка посередине
    while (*str)
	screen.overlay[p++]=r_u8(&xlat[(uint8_t)*str++]);
    
    // Пустое место справа
    while (p < 64)
	screen.overlay[p++]=0;
    
    // Таймер на 1 секунду
    screen.overlay_timer=50;	// 50 полей - 1 секунда
}


void vg75_W(uint8_t A, uint8_t value)
{
    if (A)
    {
	// Команда
	vg75.cmd=value;
	vg75.param_n=0;
	
	if ( (value & 0xE0) == 0x20 )
	{
	    // Включение/отключение дисплея
	    if ( ((value>>2) & 0x07) != 0 )
	    {
		// Включить
		screen.dma_burst=(1 << (value & 0x03));
		//ets_printf("VG75: start display dma_burst=%d\n", screen.dma_burst);
		sreg|=SREG_VE;
	    } else
	    {
		// Отключить
		//ets_printf("VG75: stop display\n");
		sreg&=~SREG_VE;
	    }
	} else
	if (value == 0xA0)
	{
	    // Разрешить прерывания
	    //ets_printf("VG75: int enable\n");
	    sreg|=SREG_IE;
	} else
	if (value == 0xC0)
	{
	    // Запретить прерывания
	    //ets_printf("VG75: int disable\n");
	    sreg&=~SREG_IE;
	}
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
	    screen.underline_y=(vg75.param[2] >> 4);	// позиция подчеркивания
	    screen.char_h=(vg75.param[2] & 0x0F)+1;	// высота символа в пикселах -1
	    screen.cursor_type=(vg75.param[3] >> 4) & 0x03;	// форма курсора: 0=мигающий блок, 1=мигающий штрих, 2=немигающий блок, 3=немигающий штрих
	    screen.attr_visible=((vg75.param[3] & 0x40) != 0);	// видимость аттрибутов (если 1, то аттрибут будет отображен пустым местом, если 0 - будет пропущен)
	    ets_printf("VG75: W=%d H=%d CH=%d CUR=%d\n", screen.screen_w, screen.screen_h, screen.char_h, screen.cursor_type);
	    if (screen.screen_w > 100) screen.screen_w=100;
	    //if (screen.screen_h > 38) screen.screen_h=38;
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
    if (A)
    {
	// SREG
	uint8_t value=sreg;
	
	// Сбрасываем биты, которые должны сброситься при чтении
	sreg&=~(SREG_IR | SREG_LP | SREG_IC | SREG_DU | SREG_FO);
	
	return value;
    } else
    {
	// PREG
	return 0x00;
    }
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
