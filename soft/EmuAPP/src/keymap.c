#include "keymap.h"

#include "ps2.h"
#include "vv55_i.h"
#include "pt/pt.h"
#include "pt.h"
#include "align4.h"


struct kmap
{
    uint16_t ps2;
    uint16_t rk;
};


// Раскладка для символов с нажатым Shift (где установлен RK_SS надо отжать СС)
static const struct kmap AT_IRAM kb_shift[]=
{
    { PS2_EQUALS,	RK_SEMICOLON	},	// '+' (Shift + '=' => СС + ';')
    { PS2_QUOTE,	RK_2		},	// '"' (Shift + ''' => CC + '2')
    { PS2_2,		RK_AT | RK_SS	},	// '@' (Shift + '2' => '@')
    { PS2_6,		RK_CARET | RK_SS},	// '^' (Shift + '6' => '^')
    { PS2_7,		RK_6		},	// '&' (Shift + '7' => CC + '6')
    { PS2_8,		RK_STAR		},	// '*' (Shift + '8' => СС + '*')
    { PS2_9,		RK_8		},	// '(' (Shift + '9' => CC + '8')
    { PS2_0,		RK_9		},	// ')' (Shift + '0' => CC + '9')
    { PS2_SEMICOLON,	RK_STAR | RK_SS	},	// ':' (Shift + ';' => '*')
    { PS2_COMMA,	RK_COMMA	},	// '<' (Shift + ',' => СС + ',')
    { PS2_PERIOD,	RK_PERIOD	},	// '>' (Shift + '.' => СС + '.')
    
    { 0, 		0			}	// end
};


// Раскладка для русских букв (включен РУС без СС или ЛАТ с СС)
static const struct kmap AT_IRAM kb_rus[]=
{
    { PS2_Q,		RK_J		},	// 'Й'
    { PS2_W,		RK_C		},	// 'Ц'
    { PS2_E,		RK_U		},	// 'У'
    { PS2_R,		RK_K		},	// 'К'
    { PS2_T,		RK_E		},	// 'Е'
    { PS2_Y,		RK_N		},	// 'Н'
    { PS2_U,		RK_G		},	// 'Г'
    { PS2_I,		RK_L_BRACKET	},	// 'Ш'
    { PS2_O,		RK_R_BRACKET	},	// 'Щ'
    { PS2_P,		RK_Z		},	// 'З'
    { PS2_L_BRACKET,	RK_H		},	// 'Х'
    { PS2_R_BRACKET,	RK_X		},	// 'Ь' ('Ъ' - нет)
    
    { PS2_A,		RK_F		},	// 'Ф'
    { PS2_S,		RK_Y		},	// 'Ы'
    { PS2_D,		RK_W		},	// 'В'
    { PS2_F,		RK_A		},	// 'А'
    { PS2_G,		RK_P		},	// 'П'
    { PS2_H,		RK_R		},	// 'Р'
    { PS2_J,		RK_O		},	// 'О'
    { PS2_K,		RK_L		},	// 'Л'
    { PS2_L,		RK_D		},	// 'Д'
    { PS2_SEMICOLON,	RK_V		},	// 'Ж'
    { PS2_QUOTE,	RK_BACK_SLASH	},	// 'Э'
    
    { PS2_Z,		RK_Q		},	// 'Я'
    { PS2_X,		RK_CARET	},	// 'Ч'
    { PS2_C,		RK_S		},	// 'С'
    { PS2_V,		RK_M		},	// 'М'
    { PS2_B,		RK_I		},	// 'И'
    { PS2_N,		RK_T		},	// 'Т'
    { PS2_M,		RK_X		},	// 'Ь'
    { PS2_COMMA,	RK_B		},	// 'Б'
    { PS2_PERIOD,	RK_AT		},	// 'Ю'
    
    { 0, 		0		}	// end
};


// Остальная раскладка (в т.ч. и ЛАТ)
static const struct kmap AT_IRAM kb_lat[]=
{
    // Ряд 1
    { PS2_SEMICOLON,	RK_SEMICOLON		},	// ';'
    { PS2_KP_PLUS,	RK_SEMICOLON | RK_SS	},	// '+'
    { PS2_1,		RK_1			},	// '1'
    { PS2_KP_1,		RK_1			},	// '1'
    { PS2_2,		RK_2			},	// '2'
    { PS2_KP_2,		RK_2			},	// '2'
    { PS2_3,		RK_3			},	// '3'
    { PS2_KP_3,		RK_3			},	// '3'
    { PS2_4,		RK_4			},	// '4'
    { PS2_KP_4,		RK_4			},	// '4'
    { PS2_5,		RK_5			},	// '5'
    { PS2_KP_5,		RK_5			},	// '5'
    { PS2_6,		RK_6			},	// '6'
    { PS2_KP_6,		RK_6			},	// '6'
    { PS2_7,		RK_7			},	// '7'
    { PS2_KP_7,		RK_7			},	// '7'
    { PS2_QUOTE,	RK_7 | RK_SS		},	// ''' => СС + '7'
    { PS2_8,		RK_8			},	// '8'
    { PS2_KP_8,		RK_8			},	// '8'
    { PS2_9,		RK_9			},	// '9'
    { PS2_KP_9,		RK_9			},	// '9'
    { PS2_0,		RK_0			},	// '0'
    { PS2_KP_0,		RK_0			},	// '0'
    { PS2_MINUS,	RK_MINUS		},	// '-'
    { PS2_KP_MINUS,	RK_MINUS		},	// '-'
    { PS2_EQUALS,	RK_MINUS | RK_SS	},	// '=' ('=' => CC + '-')
    { PS2_TAB,		RK_TAB			},	// ТАБ
    { PS2_KP_ENTER,	RK_PS			},	// ПС
    
    // Ряд 2
    { PS2_J,		RK_J			},	// 'J'
    { PS2_C,		RK_C			},	// 'C'
    { PS2_U,		RK_U			},	// 'U'
    { PS2_K,		RK_K			},	// 'K'
    { PS2_E,		RK_E			},	// 'E'
    { PS2_N,		RK_N			},	// 'N'
    { PS2_G,		RK_G			},	// 'G'
    { PS2_L_BRACKET,	RK_L_BRACKET		},	// '['
    { PS2_R_BRACKET,	RK_R_BRACKET		},	// ']'
    { PS2_Z,		RK_Z			},	// 'Z'
    { PS2_H,		RK_H			},	// 'H'
    { PS2_KP_STAR,	RK_STAR | RK_SS		},	// '*'
    { PS2_ENTER,	RK_VK			},	// ВК
    
    // Ряд 3
    { PS2_L_CTRL,	RK_US			},	// УС
    { PS2_R_CTRL,	RK_US			},	// УС
    { PS2_F,		RK_F			},	// 'Ф'
    { PS2_Y,		RK_Y			},	// 'Ы'
    { PS2_W,		RK_W			},	// 'В'
    { PS2_A,		RK_A			},	// 'А'
    { PS2_P,		RK_P			},	// 'П'
    { PS2_R,		RK_R			},	// 'Р'
    { PS2_O,		RK_O			},	// 'О'
    { PS2_L,		RK_L			},	// 'Л'
    { PS2_D,		RK_D			},	// 'Д'
    { PS2_V,		RK_V			},	// 'Ж'
    { PS2_BACK_SLASH,	RK_BACK_SLASH		},	// '\'
    { PS2_PERIOD,	RK_PERIOD		},	// '.'
    { PS2_KP_PERIOD,	RK_PERIOD		},	// '.'
    { PS2_BACKSPACE,	RK_ZB			},	// ЗБ
    
    // Ряд 4
    { PS2_L_SHIFT,	RK_SS			},	// СС
    { PS2_R_SHIFT,	RK_SS			},	// СС
    { PS2_Q,		RK_Q			},	// 'Q'
    // ^
    { PS2_S,		RK_S			},	// 'S'
    { PS2_M,		RK_M			},	// 'M'
    { PS2_I,		RK_I			},	// 'I'
    { PS2_T,		RK_T			},	// 'T'
    { PS2_X,		RK_X			},	// 'X'
    { PS2_B,		RK_B			},	// 'B'
    // @
    { PS2_COMMA,	RK_COMMA		},	// ','
    { PS2_SLASH,	RK_SLASH		},	// '/'
    { PS2_KP_SLASH,	RK_SLASH		},	// '/'
    { PS2_CAPS,		RK_RL			},	// РУС/ЛАТ
    
    // Ряд 5
    { PS2_SPACE,	RK_SPACE		},	// 
    
    // Правая часть
    { PS2_F1,		RK_F1			},	// Ф1
    { PS2_F2,		RK_F2			},	// Ф2
    { PS2_F3,		RK_F3			},	// Ф3
    { PS2_F4,		RK_F4			},	// Ф4
    { PS2_LEFT,		RK_LEFT			},	// <--
    { PS2_RIGHT,	RK_RIGHT		},	// -->
    { PS2_UP,		RK_UP			},	// Вверх
    { PS2_DOWN,		RK_DOWN			},	// Вниз
    { PS2_HOME,		RK_HOME			},	// \ (стрелка влево-вверх)
    { PS2_DELETE,	RK_STR			},	// СТР
    { PS2_END,		RK_STR			},	// СТР
    { PS2_L_ALT,	RK_AR2			},	// АР2
    { PS2_R_ALT,	RK_AR2			},	// АР2
    
    { 0, 		0			}	// end
};


static uint16_t code, unhandled_code=0;
static const struct kmap *map;


static PT_THREAD(handle_code(struct pt *pt))
{
    static uint8_t n;
    static uint32_t _sleep;
    static uint16_t c, r;
    
    PT_BEGIN(pt);
	n=0;
	while ( (c=r_u16(&map[n].ps2)) != 0 )
	{
	    if (c == (code & 0x7fff))
	    {
		// Нашли
		r=r_u16(&map[n].rk);
		if (r & RK_SS)
		{
		    // Нужно эмулировать нажатие/отжатие СС + кнопку
		    if (r==RK_SS)
		    {
			// Просто СС
			if (code & 0x8000)
			    kbd_releaseAll(r); else
			    kbd_press(r);
		    } else
		    {
			// Эмуляция нажатия/отжатия СС + кнопка
			if (! (code & 0x8000))
			{
#define KEY_T	50
			    // Меняем состояние СС
			    if (kbd_ss())
				kbd_release(RK_SS); else
				kbd_press(RK_SS);
			    
			    // Нажимаем кнопку
			    kbd_press(r & 0xFFF);
			    
			    // Ждем
			    PT_SLEEP(KEY_T*1000);
			    
			    // Отжимаем кнопку
			    kbd_releaseAll(0);
			    
			    // Меняем состояние СС обратно
			    if (kbd_ss())
				kbd_release(RK_SS); else
				kbd_press(RK_SS);
#undef KEY_T
			}
		    }
		} else
		{
		    // Просто кнопка
		    if (code & 0x8000)
		        kbd_release(r); else
		        kbd_press(r);
		}
		
		// Обнуляем код - значит что обработали
		code=0;
		break;
	    }
	    
	    // Следующий код
	    n++;
	}
	if (0) PT_YIELD(pt);	// отключить warning
    PT_END(pt);
}


static PT_THREAD(task(struct pt *pt))
{
    static struct pt sub;
    
    PT_BEGIN(pt);
	while (1)
	{
	    // Получаем скан-код
	    code=ps2_read();
	    if (!code)
	    {
		// Нет нажатия
		PT_YIELD(pt);
		continue;
	    }
	    
	    if (kbd_ss())
	    {
		// Отдельно обрабатываем кнопки с нажатым Shift, чтобы они совпадали с современной клавиатурой
		map=kb_shift;
		PT_SPAWN(pt, &sub, handle_code(&sub));
		if (code==0) continue;
	    }
	    
	    if ( ( (kbd_rus()) && (! kbd_ss()) ) ||
		 ( (! kbd_rus()) && (kbd_ss()) ) )
	    {
		// Отдельно обрабатываем русские буквы, чтобы они совпадали с современной клавиатурой
		map=kb_rus;
		PT_SPAWN(pt, &sub, handle_code(&sub));
		if (code==0) continue;
	    }
	    
	    // Оставшиеся символы
	    map=kb_lat;
	    PT_SPAWN(pt, &sub, handle_code(&sub));
	    if (code==0) continue;
	    
	    // Код не обработан
	    unhandled_code=code;
	}
    PT_END(pt);
}


static struct pt pt_task;


void keymap_init(void)
{
    PT_INIT(&pt_task);
}


uint16_t keymap_periodic(void)
{
    (void)PT_SCHEDULE(task(&pt_task));
    
    uint16_t ret=unhandled_code;
    unhandled_code=0;
    
    return ret;
}
