#include "ps2_codes.h"

#include "ets.h"
#include "ps2.h"
#include "align4.h"


struct map
{
    uint16_t ps2;
    char sym;
};

static struct map AT_IRAM kmap[] =
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
    
    { PS2_BACKSPACE,	8 },
    { PS2_ENTER,	'\n' },
    { PS2_KP_ENTER,	'\n' },
    { PS2_ESC,		'x' },
    { PS2_UP,		'u' },
    { PS2_DOWN,		'd' },
    { PS2_LEFT,		'l' },
    { PS2_RIGHT,	'r' },
    
    { 0,		0 },
};


char ps2_sym(void)
{
    uint16_t code=ps2_read();
    
    if (code==0) return 0;
    
    uint8_t n=0;
    uint16_t c;
    while ( (c=r_u16(&kmap[n].ps2)) != 0 )
    {
	if (c==code) return r_u8((const uint8_t*)&kmap[n].sym);
	n++;
    }
    return 0;
}
