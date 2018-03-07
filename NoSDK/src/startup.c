#include "ets.h"


extern uint32_t _bss_start[], _bss_end[];
extern void main_program(void);


static void my_putc1(char c)
{
    // У UART есть FIFO на 128 байт, а ждать нам при эмуляции совсем не нужно
    WRITE_PERI_REG(/*UART_FIFO(0)*/ 0x60000000, c);
}


void call_user_start(void)
{
    if(rom_i2c_readReg(103,4,1) != 136) // 8: 40MHz, 136: 26MHz
    {
        //if(get_sys_const(sys_const_crystal_26m_en) == 1) // soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz
        {
            // set 80MHz PLL CPU
            rom_i2c_writeReg(103,4,1,0x88);
            rom_i2c_writeReg(103,4,2,0x91);
        }
    }

    // Переключаемся на 160МГц
    REG_SET_BIT(0x3ff00014, BIT(0));
    ets_update_cpu_frequency(160);
    
    // Инитим UART на 115200
    uart_div_modify(0, 80000000/115200);
    
    // Инитим BSS
    uint8_t *ptr=(uint8_t*)_bss_start;
    while (ptr < (uint8_t*)_bss_end)
	(*ptr++)=0;
    
    // Заменяем обработчик putc1
    ets_install_putc1((void*)my_putc1);
    
    
    // Запускаем основную программу
    main_program();
}
