#include "ps2.h"

#include <osapi.h>
#include <gpio.h>
#include "gpio_lib.h"
#include "timer0.h"
#include "pt_sleep.h"


//http://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf


// NodeMCU
//#define DATA	13
//#define CLK	12

// ESP-01
#define DATA	2
#define CLK	0


#define RXQ_SIZE	16


static uint32_t prev_T=0;
static uint16_t rxq[RXQ_SIZE];
static uint8_t rxq_head=0, rxq_tail=0;

static uint16_t tx=0, txbit=0;
static uint8_t led_status;
static bool ack=0;

static struct pt pt_task;


static void RAMFUNC gpio_int(void *arg)
{
    static uint16_t rx=0, rxbit=1;
    static bool was_E0=0, was_E1=0, was_F0=0;
    
    uint32_t gpio_status;
    gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
    
    // Получаем время от прошлого бита, если оно слишком большое - сбросим автомат приема
    uint32_t T=getCycleCount();
    uint32_t dT=T-prev_T;
    if (dT < 8000) return;	// если фронт короче 50мкс (по стандарту 60-100мкс), то это иголка (пропускаем ее)
    if (dT > 19200)	// 120мкс таймаут
    {
	// Сбрасываем приемник
	rx=0;
	rxbit=1;
    }
    prev_T=T;
    
    if (txbit)
    {
	if (txbit & (1<<9))
	{
	    // Передали 8 бит данных и 1 бит четности - переходим в прием
	    tx=0;
	    txbit=0;
	    gpio_init_input_pu(DATA);
	} else
	{
	    // Мы в режиме передачи
	    if (tx & txbit)
		gpio_on(DATA); else
		gpio_off(DATA);
	    txbit<<=1;
	}
    } else
    {
	// Мы в режиме приема
	
	// Принимаем бит
	if (gpio_in(DATA)) rx|=rxbit;
	rxbit<<=1;
	
	// Проверяем на конец байта
	if (rxbit & (1<<11))
	{
	    // Принято 11 бит
	    if ( (!(rx & 0x001)) && (rx & 0x400) )	// проверим наличие старт и стоп битов
	    {
		// Убираем стартовый бит
		rx>>=1;
		
		// Получаем код
		uint8_t code=rx & 0xff;
		
		// Считаем четность
		rx^=rx >> 4;
		rx^=rx >> 2;
		rx^=rx >> 1;
		rx^=rx >> 8;
		
		if (! (rx & 1))
		{
		    // Все нормально !
		    if (code==0xE0) was_E0=1; else
		    if (code==0xE1) was_E1=1; else
		    if (code==0xF0) was_F0=1; else
		    if (code==0xFA) ack=1; else
		    {
			uint16_t code16=code;
			
			// Расширенные наборы
			if (was_E0) code16|=0x0100; else
			if (was_E1) code16|=0x0200;
			
			// Отжатие
			if (was_F0) code16|=0x8000;
			
			// Кладем в буфер
			rxq[rxq_head]=code16;
			rxq_head=(rxq_head + 1) & (RXQ_SIZE-1);
			
			// Сбрасываем флаги
			was_E0=was_E1=was_F0=0;
		    }
		}
	    }
	    
	    // Сбрасываем приемник
	    rx=0;
	    rxbit=1;
	}
    }
}


void ps2_init(void)
{
    // Переключаем DATA и CLK в GPIO
    gpio_init_input_pu(DATA);
    gpio_init_input_pu(CLK);
    
    // Настраиваем прерывание по низкому фронту на CLK
    gpio_pin_intr_state_set(GPIO_ID_PIN(CLK), GPIO_PIN_INTR_NEGEDGE);
    
    // Настраиваем прерывание по GPIO
    ETS_GPIO_INTR_ATTACH(gpio_int, 0);
    ETS_GPIO_INTR_ENABLE();
    
    // Задача периодики
    PT_INIT(&pt_task);
}


uint16_t ps2_read(void)
{
    if (rxq_head == rxq_tail) return 0;
    uint16_t d=rxq[rxq_tail];
    rxq_tail=(rxq_tail + 1) & (RXQ_SIZE-1);
    return d;
}


void ps2_leds(bool caps, bool num, bool scroll)
{
    led_status=(caps ? 0x04 : 0x00) | (num ? 0x02 : 0x00) | (scroll ? 0x01 : 0x00);
}


static void start_tx(uint8_t b)
{
    uint8_t p=b;
    p^=p >> 4;
    p^=p >> 2;
    p^=p >> 1;
    tx=b | ((p & 0x01) ? 0x000 : 0x100);
    txbit=1;
}


static PT_THREAD(task(struct pt *pt))
{
    static uint32_t _sleep;
    static uint8_t last_led=0x00;
    static uint8_t l;
    
    PT_BEGIN(pt);
	while (1)
	{
	    if (last_led == led_status)
	    {
		// Лампочки не изменились
		PT_YIELD(pt);
		continue;
	    }
	    
	    
	    // CLK вниз
	    gpio_off(CLK);
	    gpio_init_output(CLK);
	    PT_SLEEP(pt, 100);
	    
	    // DATA вниз (старт бит)
	    gpio_off(DATA);
	    gpio_init_output(DATA);
	    PT_SLEEP(pt, 200);
	    
	    // Отправляем команду "Set/Reset LEDs"
	    ack=0;
	    start_tx(0xED);
	    
	    // Отпускаем CLK
	    gpio_init_input_pu(CLK);
	    
	    // Ждем немного
	    PT_SLEEP(pt, 5000);
	    
	    // Проверим подтверждение
	    if (! ack) continue;
	    
	    
	    
	    // CLK вниз
	    gpio_off(CLK);
	    gpio_init_output(CLK);
	    PT_SLEEP(pt, 100);
	    
	    // DATA вниз (старт бит)
	    gpio_off(DATA);
	    gpio_init_output(DATA);
	    PT_SLEEP(pt, 200);
	    
	    // Отправляем лампочки
	    ack=0;
	    l=led_status;
	    start_tx(l);
	    
	    // Отпускаем CLK
	    gpio_init_input_pu(CLK);
	    
	    // Ждем немного
	    PT_SLEEP(pt, 5000);
	    
	    // Проверим подтверждение
	    if (! ack) continue;
	    
	    // Сохраняем отправленное состояние
	    last_led=l;
	}
    PT_END(pt);
}


void ps2_periodic(void)
{
    (void)PT_SCHEDULE(task(&pt_task));
}
