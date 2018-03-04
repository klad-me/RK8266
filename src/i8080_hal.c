#include "i8080_hal.h"
#include <string.h>
#include <osapi.h>

#include "vg75.h"
#include "kbd.h"
#include "rom.h"


uint8_t RAM[0x8000];


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
	// ïúõ
	return RAM[addr & 0x7fff];
    } else
    {
	// ðÅÒÅÆÅÒÉÑ/ðúõ
	switch ((addr >> 12) & 0x0f)
	{
	    case 0x8:
	    case 0x9:
		//os_printf("KBD: read %04x\n", addr);
		return kbd_R(addr & 0x03);
	    
	    case 0xA:
	    case 0xB:
		//os_printf("ParPort: read %04x\n", addr);
		return 0x00;
	    
	    case 0xC:
	    case 0xD:
		// ÷ç75
		//os_printf("VG75: read %04x\n", addr);
		return vg75_R(addr & 1);
	    
	    case 0xE:
		//os_printf("DMA: read %04x\n", addr);
		return ik57_R(addr & 0x0f);
	    
	    case 0xF:
		// ðúõ
		return ROM[addr & 0x07ff];
	    
	    default:
		return 0x00;
	}
    }
}


void i8080_hal_memory_write_byte(int addr, int byte)
{
    if ( (addr & 0x8000) == 0 )
    {
	// ïúõ
	RAM[addr & 0x7fff]=byte;
    } else
    {
	// ðÅÒÅÆÅÒÉÑ
	switch ((addr >> 12) & 0x0f)
	{
	    case 0x8:
	    case 0x9:
		kbd_W(addr & 0x03, byte);
		break;
	    
	    case 0xA:
	    case 0xB:
		//os_printf("ParPort: %04x=%02x\n", addr, (uint8_t)byte);
		break;
	    
	    case 0xC:
	    case 0xD:
		//os_printf("VG75: %04x=%02x\n", addr, (uint8_t)byte);
		vg75_W(addr & 1, byte);
		break;
	    
	    case 0xE:
		//os_printf("DMA: %04x=%02x\n", addr, (uint8_t)byte);
		ik57_W(addr & 0x0f, byte);
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
    memset(RAM, 0x00, sizeof(RAM));
}
