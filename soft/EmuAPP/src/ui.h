#ifndef UI_H
#define UI_H


#include "ets.h"


extern char ui_scr[38][80];


void ui_clear(void);
void ui_header(const char *s);
void ui_draw_list(const char *s);
void ui_draw_text(uint8_t x, uint8_t y, const char *s);
int8_t ui_select(uint8_t count);
const char* ui_input_text(const char *comment, const char *text, uint8_t max_len);
int8_t ui_yes_no(const char *comment);

void ui_start(void);
void ui_stop(void);

void ui_sleep(uint16_t ms);


#endif
