#include "ps2.h"

#include <osapi.h>
#include <gpio.h>
#include "gpio_lib.h"
#include "timer0.h"
#include "board.h"

//http://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf


#define RXQ_SIZE	16


static uint32_t prev_T=0;
static uint16_t rxq[RXQ_SIZE];
static uint8_t rxq_head=0, rxq_tail=0;


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
    
    {
	// Мы в режиме приема
	
	// Принимаем бит
	if (gpio_in(PS2_DATA)) rx|=rxbit;
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
    // Переключаем PS2_DATA и PS2_CLK в GPIO
    gpio_init_input_pu(PS2_DATA);
    gpio_init_input_pu(PS2_CLK);
    
    // Настраиваем прерывание по низкому фронту на PS2_CLK
    gpio_pin_intr_state_set(GPIO_ID_PIN(PS2_CLK), GPIO_PIN_INTR_NEGEDGE);
    
    // Настраиваем прерывание по GPIO
    ETS_GPIO_INTR_ATTACH(gpio_int, 0);
    ETS_GPIO_INTR_ENABLE();
}


uint16_t ps2_read(void)
{
    if (rxq_head == rxq_tail) return 0;
    uint16_t d=rxq[rxq_tail];
    rxq_tail=(rxq_tail + 1) & (RXQ_SIZE-1);
    return d;
}
