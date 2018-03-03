#include "kbd.h"

#include <osapi.h>


#define MOD_SS	0x20
#define MOD_US	0x40
#define MOD_RL	0x80


static uint8_t scanline=0x00, mods=0xff;
static uint8_t kbd[8];


void kbd_init(void)
{
    os_memset(kbd, 0xff, sizeof(kbd));
}


void kbd_press(uint16_t code)
{
    if (code >> 12)
    {
	// Модификаторы
        mods&=~((code >> 8) & 0xf0);
    } else
    {
	// Скан-матрица
    	kbd[ (code >> 8) & 0x07 ]&=~(code & 0xff);
    }
}


void kbd_release(uint16_t code)
{
    if (code >> 12)
    {
	// Модификаторы
        mods|=((code >> 8) & 0xf0);
    } else
    {
	// Скан-матрица
        kbd[ (code >> 8) & 0x07 ]|=(code & 0xff);
    }
}


void kbd_W(uint8_t A, uint8_t value)
{
    if (A==0x00) scanline=value;
    //printf("KBD A=%02x value=%02x\n", A, value);
}


uint8_t kbd_R(uint8_t A)
{
    switch (A)
    {
	case 0:
	    // Канал A: выход
	    return scanline;
	
	case 1:
	    // Канал B: вход
	    {
		uint8_t scan=~scanline, i, v=0xff;
		for (i=0; i<8; i++)
		    if (scan & (1<<i))
			v&=kbd[i];
		return v;
	    }
	    break;
	
	case 2:
	    // Канал C: модификаторы
	    return mods;
	
	case 3:
	default:
	    return 0x00;
    }
}
