#include "i8080_hal.h"

#include "vg75.h"
#include "zkg.h"
#include "vv55_i.h"
#include "rom.h"
#include "align4.h"


uint8_t RAM[0x8000];
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
		// ВВ55 внешняя
		//os_printf("ParPort: read %04x\n", addr);
		return 0x00;
	    
	    case 0xC:
	    case 0xD:
		// ВГ75
		return vg75_R(addr & 1);
	    
	    case 0xE:
		// ИК57
		return ik57_R(addr & 0x0f);
	    
	    case 0xF:
		// ПЗУ
		return r_u8(&ROM[addr & 0x07ff]);
	    
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
		// ВВ55 внешняя
		//os_printf("ParPort: %04x=%02x\n", addr, (uint8_t)byte);
		break;
	    
	    case 0xC:
	    case 0xD:
		vg75_W(addr & 1, byte);
		break;
	    
	    case 0xE:
		// ИК57
		ik57_W(addr & 0x0f, byte);
		break;
	    
	    case 0xF:
		// Знакогенератор
		{
		    addr&=0x3ff;
		    
		    // Меняем адресацию
		    uint8_t c=(addr >> 3);
		    uint8_t l=(addr & 0x07);
		    zkg[ (l<<7) + c ]=(byte ^ 0xff) & 0x3F;	// записываем с инверсией
		}
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
}


unsigned char* i8080_hal_memory(void)
{
    return RAM;
}


void i8080_hal_init(void)
{
    ets_memset(RAM, 0x00, sizeof(RAM));
}
