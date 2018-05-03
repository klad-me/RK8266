#include "i8080_hal.h"

#include "vg75.h"
#include "zkg.h"
#include "vv55_i.h"
#include "rom.h"
#include "align4.h"
#include "gpio_lib.h"
#include "board.h"


uint8_t RAM[0x8000], RAM2[0x2000];
uint8_t *ROM=(uint8_t*)(0x40110000-0x2000);	// верх IRAM
uint32_t i8080_cycles;


int i8080_hal_memory_read_word(int addr)
{
    return 
        (i8080_hal_memory_read_byte(addr + 1) << 8) |
        i8080_hal_memory_read_byte(addr);
}


void i8080_hal_memory_write_word(int addr, int word)
{
    i8080_hal_memory_write_byte(addr, word & 0xff);
    i8080_hal_memory_write_byte(addr + 1, (word >> 8) & 0xff);
}


int i8080_hal_memory_read_byte(int addr)
{
    if ( (addr & 0x8000) == 0 )
    {
	// ОЗУ
	return RAM[addr & 0x7fff];
    } else
    {
	// Переферия/ПЗУ
	switch ((addr >> 12) & 0x0f)
	{
	    case 0x8:
	    case 0x9:
		// ВВ55 внутренняя
		return vv55_i_R(addr & 0x03);
	    
	    case 0xA:
	    case 0xB:
		// Доп.ОЗУ вместо ВВ55
		return RAM2[addr & 0x1FFF];
	    
	    case 0xC:
	    case 0xD:
		// ВГ75 + шрифты
		if (addr & (1 << 10))	// A10 - переключатель ВГ75/шрифт
		{
		    // Шрифт (A12,A11 - номер шрифта)
		    uint8_t n=(addr >> 11) & 0x03;
		    addr&=0x3FF;
		    addr=((addr & 0x07) << 7) | (addr >> 3);	// меняем адресацию
		    return zkg[n][addr] ^ 0xFF;
		} else
		{
		    // ВГ75
		    return vg75_R(addr & 1);
		}
	    
	    case 0xE:
	    case 0xF:
		// ПЗУ вместо ИК57 (ИК57 никто не читает)
		return r_u8(&ROM[addr & 0x1FFF]);
	    
	    default:
		return 0x00;
	}
    }
}


void i8080_hal_memory_write_byte(int addr, int byte)
{
    if ( (addr & 0x8000) == 0 )
    {
	// ОЗУ
	RAM[addr & 0x7fff]=byte;
    } else
    {
	// Переферия
	switch ((addr >> 12) & 0x0f)
	{
	    case 0x8:
	    case 0x9:
		// ВВ55 внутренняя
		vv55_i_W(addr & 0x03, byte);
		break;
	    
	    case 0xA:
	    case 0xB:
		// Доп.ОЗУ вместо ВВ55
		RAM2[addr & 0x1FFF]=byte;
		break;
	    
	    case 0xC:
	    case 0xD:
		// ВГ75 + шрифты
		if (addr & (1 << 10))	// A10 - переключатель ВГ75/шрифт
		{
		    // Шрифт (A12,A11 - номер шрифта)
		    uint8_t n=(addr >> 11) & 0x03;
		    addr&=0x3FF;
		    addr=((addr & 0x07) << 7) | (addr >> 3);	// меняем адресацию
		    if (n!=0) zkg[n][addr]=byte ^ 0xFF;	// 0-й шрифт запрещаем менять
		} else
		{
		    // ВГ75
		    vg75_W(addr & 1, byte);
		}
		break;
	    
	    case 0xE:
		// ИК57
		ik57_W(addr & 0x0f, byte);
		break;
	    
	    case 0xF:
		// ПЗУ - записывать нельзя
		break;
	}
    }
}


int i8080_hal_io_input(int port)
{
    return 0;
}


void i8080_hal_io_output(int port, int value)
{
}


void i8080_hal_iff(int on)
{
    if (on) gpio_on(BEEPER); else gpio_off(BEEPER);
}


unsigned char* i8080_hal_memory(void)
{
    return RAM;
}


unsigned char* i8080_hal_rom(void)
{
    return ROM;
}


void i8080_hal_init(void)
{
    // Инитим ОЗУ
    ets_memset(RAM, 0x00, sizeof(RAM));
    ets_memset(RAM2, 0x00, sizeof(RAM2));
    
    // Инитим ПЗУ
    ets_memset(ROM+0x0000, 0xFF, 0x1800);
    ets_memcpy(ROM+0x1800, ROM_F800, 0x8000);
    
    // Инитим порт пищалки
    gpio_init_output(BEEPER);
    gpio_off(BEEPER);
}
