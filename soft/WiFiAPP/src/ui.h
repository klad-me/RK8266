#ifndef UI_H
#define UI_H


#include <os_type.h>
#include "pt/pt.h"


extern int8_t ui_select_n, ui_select_count;


void ui_clear(void);
void ui_header(const char *s);
void ui_draw_list(const char *s);
void ui_draw_text(uint8_t x, uint8_t y, const char *s);
PT_THREAD(ui_select(struct pt *pt));

void ui_start(void);


#endif
