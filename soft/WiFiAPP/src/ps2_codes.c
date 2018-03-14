#include "ps2_codes.h"

#include "ps2.h"
#include <osapi.h>


struct map
{
    uint16_t ps2;
    char sym;
};

static struct map kmap_shift[] =
{
    { PS2_A,		'A' },
    { PS2_B,		'B' },
    { PS2_C,		'C' },
    { PS2_D,		'D' },
    { PS2_E,		'E' },
    { PS2_F,		'F' },
    { PS2_G,		'G' },
    { PS2_H,		'H' },
    { PS2_I,		'I' },
    { PS2_J,		'J' },
    { PS2_K,		'K' },
    { PS2_L,		'L' },
    { PS2_M,		'M' },
    { PS2_N,		'N' },
    { PS2_O,		'O' },
    { PS2_P,		'P' },
    { PS2_Q,		'Q' },
    { PS2_R,		'R' },
    { PS2_S,		'S' },
    { PS2_T,		'T' },
    { PS2_U,		'U' },
    { PS2_V,		'V' },
    { PS2_W,		'W' },
    { PS2_X,		'X' },
    { PS2_Y,		'Y' },
    { PS2_Z,		'Z' },
    
    { PS2_ACCENT,	'~' },
    { PS2_1,		'!' },
    { PS2_2,		'@' },
    { PS2_3,		'#' },
    { PS2_4,		'$' },
    { PS2_5,		'%' },
    { PS2_6,		'^' },
    { PS2_7,		'&' },
    { PS2_8,		'*' },
    { PS2_9,		'(' },
    { PS2_0,		')' },
    { PS2_MINUS,	'_' },
    { PS2_EQUALS,	'+' },
    
    { PS2_L_BRACKET,	'{' },
    { PS2_R_BRACKET,	'}' },
    { PS2_SEMICOLON,	':' },
    { PS2_QUOTE,	'"' },
    { PS2_BACK_SLASH,	'|' },
    { PS2_COMMA,	'<' },
    { PS2_PERIOD,	'>' },
    { PS2_SLASH,	'?' },
    
    { 0,		0 },
};


static struct map kmap[] =
{
    { PS2_A,		'a' },
    { PS2_B,		'b' },
    { PS2_C,		'c' },
    { PS2_D,		'd' },
    { PS2_E,		'e' },
    { PS2_F,		'f' },
    { PS2_G,		'g' },
    { PS2_H,		'h' },
    { PS2_I,		'i' },
    { PS2_J,		'j' },
    { PS2_K,		'k' },
    { PS2_L,		'l' },
    { PS2_M,		'm' },
    { PS2_N,		'n' },
    { PS2_O,		'o' },
    { PS2_P,		'p' },
    { PS2_Q,		'q' },
    { PS2_R,		'r' },
    { PS2_S,		's' },
    { PS2_T,		't' },
    { PS2_U,		'u' },
    { PS2_V,		'v' },
    { PS2_W,		'w' },
    { PS2_X,		'x' },
    { PS2_Y,		'y' },
    { PS2_Z,		'z' },
    
    { PS2_0,		'0' },
    { PS2_1,		'1' },
    { PS2_2,		'2' },
    { PS2_3,		'3' },
    { PS2_4,		'4' },
    { PS2_5,		'5' },
    { PS2_6,		'6' },
    { PS2_7,		'7' },
    { PS2_8,		'8' },
    { PS2_9,		'9' },
    
    { PS2_KP_0,		'0' },
    { PS2_KP_1,		'1' },
    { PS2_KP_2,		'2' },
    { PS2_KP_3,		'3' },
    { PS2_KP_4,		'4' },
    { PS2_KP_5,		'5' },
    { PS2_KP_6,		'6' },
    { PS2_KP_7,		'7' },
    { PS2_KP_8,		'8' },
    { PS2_KP_9,		'9' },
    
    { PS2_ACCENT,	'`' },
    { PS2_MINUS,	'-' },
    { PS2_EQUALS,	'=' },
    { PS2_BACK_SLASH,	'\\' },
    { PS2_SPACE,	' ' },
    { PS2_TAB,		'\t' },
    { PS2_L_BRACKET,	'[' },
    { PS2_R_BRACKET,	']' },
    { PS2_SEMICOLON,	';' },
    { PS2_QUOTE,	'\'' },
    { PS2_COMMA,	',' },
    { PS2_PERIOD,	'.' },
    { PS2_SLASH,	'/' },
    
    { PS2_BACKSPACE,	KEY_BACKSPACE },
    { PS2_ENTER,	KEY_ENTER },
    { PS2_KP_ENTER,	KEY_ENTER },
    { PS2_ESC,		KEY_ESC },
    { PS2_UP,		KEY_UP },
    { PS2_DOWN,		KEY_DOWN },
    { PS2_LEFT,		KEY_LEFT },
    { PS2_RIGHT,	KEY_RIGHT },
    
    { 0,		0 },
};


static char sym(uint16_t code, struct map *m)
{
    uint8_t n=0;
    while (m[n].ps2)
    {
	if (m[n].ps2==code) return m[n].sym;
	n++;
    }
    return 0;
}


char ps2_sym(void)
{
    static bool shift=false;
    uint16_t code;
    
    while ( (code=ps2_read()) != 0 )
    {
	if ( ((code & 0x7fff) == PS2_L_SHIFT) ||
	     ((code & 0x7fff) == PS2_R_SHIFT) )
	{
	    // Нажат или отжат Shift
	    shift=((code & 0x8000) == 0);
	}
	
	char c;
	
	if (shift)
	{
	    c=sym(code, kmap_shift);
	    if (c) return c;
	}
	
	c=sym(code, kmap);
	if (c) return c;
    }
    
    return 0;
}
