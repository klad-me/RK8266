#ifndef TV_DATA_H
#define TV_DATA_H


#include <c_types.h>


#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t screen_w, screen_h, char_h;
extern uint8_t cursor_x, cursor_y, cursor_type;
extern uint8_t *vram;


void tv_data_init(void);
uint8_t* tv_data_line(void);
void tv_data_periodic(void);


#ifdef __cplusplus
};
#endif


#endif
