#ifndef UI_H
#define UI_H


#include "ets.h"
#include "pt.h"


extern char ui_scr[38][78];
extern int8_t ui_select_n, ui_select_count;


void ui_clear(void);
void ui_header(const char *s);
void ui_draw_list(const char *s);
void ui_draw_text(uint8_t x, uint8_t y, const char *s);
PT_THREAD(ui_select(struct pt *pt));

void ui_start(void);


#endif
