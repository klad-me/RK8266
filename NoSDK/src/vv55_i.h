#ifndef VV55_I_H
#define VV55_I_H


#include "ets.h"


#ifdef __cplusplus
extern "C" {
#endif


// Модификаторы
#define MOD_SS		0x20
#define MOD_US		0x40
#define MOD_RL		0x80


// Строка 0
#define RK_HOME		0x0001	// стрелка влево-вверх
#define RK_STR		0x0002	// СТР
#define RK_AR2		0x0004	// АР2
#define RK_F1		0x0008	// Ф1
#define RK_F2		0x0010	// Ф2
#define RK_F3		0x0020	// Ф3
#define RK_F4		0x0040	// Ф4

// Строка 1
#define RK_TAB		0x0101	// ТАБ
#define RK_PS		0x0102	// ПС
#define RK_VK		0x0104	// ВК
#define RK_ZB		0x0108	// ЗБ
#define RK_LEFT		0x0110	// <-
#define RK_UP		0x0120	// Вверх
#define RK_RIGHT	0x0140	// ->
#define RK_DOWN		0x0180	// Вниз

// Строка 2
#define RK_0		0x0201	// 0
#define RK_1		0x0202	// 1
#define RK_2		0x0204	// 2
#define RK_3		0x0208	// 3
#define RK_4		0x0210	// 4
#define RK_5		0x0220	// 5
#define RK_6		0x0240	// 6
#define RK_7		0x0280	// 7

// Строка 3
#define RK_8		0x0301	// 8
#define RK_9		0x0302	// 9
#define RK_STAR		0x0304	// *
#define RK_SEMICOLON	0x0308	// ;
#define RK_COMMA	0x0310	// ,
#define RK_MINUS	0x0320	// -
#define RK_PERIOD	0x0340	// .
#define RK_SLASH	0x0380	// /

// Строка 4
#define RK_AT		0x0401	// @ Ю
#define RK_A		0x0402	// A А
#define RK_B		0x0404	// B Б
#define RK_C		0x0408	// C Ц
#define RK_D		0x0410	// D Д
#define RK_E		0x0420	// E Е
#define RK_F		0x0440	// F Ф
#define RK_G		0x0480	// G Г

// Строка 5
#define RK_H		0x0501	// H Х
#define RK_I		0x0502	// I И
#define RK_J		0x0504	// J Й
#define RK_K		0x0508	// K К
#define RK_L		0x0510	// L Л
#define RK_M		0x0520	// M М
#define RK_N		0x0540	// N Н
#define RK_O		0x0580	// O О

// Строка 6
#define RK_P		0x0601	// P П
#define RK_Q		0x0602	// Q Я
#define RK_R		0x0604	// R Р
#define RK_S		0x0608	// S С
#define RK_T		0x0610	// T Т
#define RK_U		0x0620	// U У
#define RK_V		0x0640	// V Ж
#define RK_W		0x0680	// W В

// Строка 7
#define RK_X		0x0701	// X Ь
#define RK_Y		0x0702	// Y Ы
#define RK_Z		0x0704	// Z З
#define RK_L_BRACKET	0x0708	// [ Ш
#define RK_BACK_SLASH	0x0710	// \ Э
#define RK_R_BRACKET	0x0720	// ] Щ
#define RK_CARET	0x0740	// ^ Ч
#define RK_SPACE	0x0780	// SPACE

// Модификаторы
#define RK_SS		0x2000	// СС
#define RK_US		0x4000	// УС
#define RK_RL		0x8000	// РУЧ/ЛАТ


void kbd_init(void);
void kbd_press(uint16_t code);
void kbd_release(uint16_t code);
void kbd_releaseAll(uint16_t code);
bool kbd_rus(void);
bool kbd_ss(void);

void kbd_dump(void);


void vv55_i_W(uint8_t A, uint8_t value);
uint8_t vv55_i_R(uint8_t A);


#ifdef __cplusplus
};
#endif


#endif
