#include "tv.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>

#include "timer0.h"
#include "gpio_lib.h"
#include "i2s.h"
#include "board.h"

#include "tv_data.h"


// Используемые паттерны для I2S
uint8_t tv_empty_line[80];
static uint8_t tv_sync_4_28__4_28[80], tv_sync_4_28__28_4[80];
static uint8_t tv_sync_28_4__4_28[80], tv_sync_28_4__28_4[80];


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


static void RAMFUNC t0_int(void)
{
    // Отключаем синхру
    gpio_off(TV_SYNC);
    timer0_write(getCycleCount()+100*160);
}


static const uint8_t* RAMFUNC tv_i2s_cb(void)
{
    static uint8_t field=0;
    static const uint8_t* *sync=TV_vsync1;
    static uint16_t line=0;
#define T_DELAY_N	6
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
    os_memset(tv_empty_line, 0x00, 80);
    tv_empty_line[3]=0xff;
    tv_empty_line[2]=0xff;
    tv_empty_line[1]=0xff;
    tv_empty_line[0]=0xff;
    tv_empty_line[7]=0xff;
    
    // Паттерн 4-28, 4-28 мкс
    os_memset(tv_sync_4_28__4_28, 0x00, 80);
    tv_sync_4_28__4_28[3]=0xff;
    tv_sync_4_28__4_28[2]=0xff;
    tv_sync_4_28__4_28[1]=0xff;
    tv_sync_4_28__4_28[0]=0xff;
    tv_sync_4_28__4_28[7]=0xff;
    tv_sync_4_28__4_28[43]=0xff;
    tv_sync_4_28__4_28[42]=0xff;
    tv_sync_4_28__4_28[41]=0xff;
    tv_sync_4_28__4_28[40]=0xff;
    tv_sync_4_28__4_28[47]=0xff;
    
    // Паттерн 4-28, 28-4 мкс
    os_memset(tv_sync_4_28__28_4, 0x00, 40);
    tv_sync_4_28__28_4[3]=0xff;
    tv_sync_4_28__28_4[2]=0xff;
    tv_sync_4_28__28_4[1]=0xff;
    tv_sync_4_28__28_4[0]=0xff;
    tv_sync_4_28__28_4[7]=0xff;
    os_memset(tv_sync_4_28__28_4+40, 0xff, 40);
    tv_sync_4_28__28_4[72]=0x00;
    tv_sync_4_28__28_4[79]=0x00;
    tv_sync_4_28__28_4[78]=0x00;
    tv_sync_4_28__28_4[77]=0x00;
    tv_sync_4_28__28_4[76]=0x00;

    // Паттерн 28-4, 4-28 мкс
    os_memset(tv_sync_28_4__4_28, 0xff, 40);
    tv_sync_28_4__4_28[32]=0x00;
    tv_sync_28_4__4_28[39]=0x00;
    tv_sync_28_4__4_28[38]=0x00;
    tv_sync_28_4__4_28[37]=0x00;
    tv_sync_28_4__4_28[36]=0x00;
    os_memset(tv_sync_28_4__4_28+40, 0x00, 40);
    tv_sync_28_4__4_28[43]=0xff;
    tv_sync_28_4__4_28[42]=0xff;
    tv_sync_28_4__4_28[41]=0xff;
    tv_sync_28_4__4_28[40]=0xff;
    tv_sync_28_4__4_28[47]=0xff;

    // Паттерн 28-4, 28-4 мкс
    os_memset(tv_sync_28_4__28_4, 0xff, 80);
    tv_sync_28_4__28_4[32]=0x00;
    tv_sync_28_4__28_4[39]=0x00;
    tv_sync_28_4__28_4[38]=0x00;
    tv_sync_28_4__28_4[37]=0x00;
    tv_sync_28_4__28_4[36]=0x00;
    tv_sync_28_4__28_4[72]=0x00;
    tv_sync_28_4__28_4[79]=0x00;
    tv_sync_28_4__28_4[78]=0x00;
    tv_sync_28_4__28_4[77]=0x00;
    tv_sync_28_4__28_4[76]=0x00;
    
    
    // Настраиваем прерывание по таймеру
    timer0_isr_init();
    timer0_attachInterrupt(t0_int);
    
    // Инитим графику
    tv_data_init();
    
    // Настраиваем I2S
    i2s_init(tv_i2s_cb, 80);
    i2s_start();
}
