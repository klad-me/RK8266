#include "tv.h"

#include "timer0.h"
#include "gpio_lib.h"
#include "i2s.h"
#include "board.h"


// Используемые паттерны для I2S
uint8_t tv_empty_line[64];
static uint8_t tv_sync_4_28__4_28[64], tv_sync_4_28__28_4[64];
static uint8_t tv_sync_28_4__4_28[64], tv_sync_28_4__28_4[64];


static const uint8_t* TV_vsync1[] =
{
    tv_sync_4_28__4_28,	// 623
    tv_sync_4_28__4_28,	// 624
    tv_sync_4_28__4_28,	// 625
    tv_sync_28_4__28_4,	// 1
    tv_sync_28_4__28_4,	// 2
    tv_sync_28_4__4_28,	// 3
    tv_sync_4_28__4_28,	// 4
    tv_sync_4_28__4_28,	// 5
    0,			// 6 - пустая строка, для выравнивания до 304 строк в поле (т.к. мы поля не различаем)
    // 7..310=304 строки - первое поле
};

static const uint8_t* TV_vsync2[] =
{
    tv_sync_4_28__4_28,	// 311
    tv_sync_4_28__4_28,	// 312
    tv_sync_4_28__28_4,	// 313
    tv_sync_28_4__28_4,	// 314
    tv_sync_28_4__28_4,	// 315
    tv_sync_4_28__4_28,	// 316
    tv_sync_4_28__4_28,	// 317
    0,			// 318
    // 319..622=304 строки - второе поле
};

static const uint8_t** TV_fields[]={TV_vsync1, TV_vsync2};


static void t0_int(void)
{
    // Отключаем синхру
    gpio_off(TV_SYNC);
    timer0_write(getCycleCount()+100*160);
}


extern void tv_data_field(void);
extern uint8_t* tv_data_line(void);


static const uint8_t* tv_i2s_cb(void)
{
    static uint8_t field=0;
    static const uint8_t* *sync=TV_vsync1;
    static uint16_t line=0;
#define T_DELAY_N	7
    static uint32_t Tdelay[T_DELAY_N];	// линия задержки синхронизации (т.к. I2S имеет задержку при выводе)
    static uint8_t Tdelay_n=0;
    
    // Включаем синхронизацию
    gpio_on(TV_SYNC);
    
    // Настраиваем таймер на отключение синхры
    Tdelay_n=(Tdelay_n+1) % T_DELAY_N;
    uint32_t *T=&Tdelay[Tdelay_n];
    timer0_write(getCycleCount()+(*T));
    
    if (line)
    {
	// Идет графика
	line--;
	(*T)=8*160;	// 2+4+2 мкс - строчная синхра
	return tv_data_line();
    } else
    {
	// Идет синхронизация
	if (*sync)
	{
	    (*T)=68*160;	// 2+64+2 мкс - кадровая синхра
	    return *sync++;
	} else
	{
	    // Конец синхронизации - даем пустую строку перед графикой
	    line=304;
	    tv_data_field();
	    
	    // Меняем поле
	    field^=1;
	    sync=TV_fields[field];
	    
	    (*T)=8*160;	// 2+4+2 мкс - строчная синхра
	    return tv_empty_line;
	}
    }
}


void tv_init(void)
{
    // Инитим порт SYNC
    gpio_init_output(TV_SYNC);
    
    // Заполняем все паттерны (порядок байт 3-2-1-0)
    
    // Пустая строка - только синхра 4мкс
    ets_memset(tv_empty_line, 0x00, 64);
    ets_memset(tv_empty_line+0, 0xff, 4);
    
    // Паттерн 4-28, 4-28 мкс
    ets_memset(tv_sync_4_28__4_28, 0x00, 64);
    ets_memset(tv_sync_4_28__4_28+0, 0xff, 4);
    ets_memset(tv_sync_4_28__4_28+32, 0xff, 4);
    
    // Паттерн 4-28, 28-4 мкс
    ets_memset(tv_sync_4_28__28_4, 0x00, 32);
    ets_memset(tv_sync_4_28__28_4+0, 0xff, 4);
    
    ets_memset(tv_sync_4_28__28_4+32, 0xff, 32);
    ets_memset(tv_sync_4_28__28_4+64-4, 0x00, 4);

    // Паттерн 28-4, 4-28 мкс
    ets_memset(tv_sync_28_4__4_28, 0xff, 32);
    ets_memset(tv_sync_28_4__4_28+32-4, 0x00, 4);
    
    ets_memset(tv_sync_28_4__4_28+32, 0x00, 32);
    ets_memset(tv_sync_28_4__4_28+64-4, 0xff, 4);

    // Паттерн 28-4, 28-4 мкс
    ets_memset(tv_sync_28_4__28_4, 0xff, 32);
    ets_memset(tv_sync_28_4__28_4+32-4, 0x00, 4);
    ets_memset(tv_sync_28_4__28_4+64-4, 0x00, 4);
}


void tv_start(void)
{
    // Настраиваем прерывание по таймеру
    timer0_isr_init();
    timer0_attachInterrupt(t0_int);
    
    // Запускаем I2S
    i2s_init(tv_i2s_cb, 64);
    i2s_start();
}
