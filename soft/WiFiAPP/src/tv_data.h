#ifndef TV_DATA_H
#define TV_DATA_H


#include <c_types.h>


#ifdef __cplusplus
extern "C" {
#endif


extern uint8_t cursor_x, cursor_y;
extern char vram[20][50];


void tv_data_init(void);
void tv_data_field(void);
uint8_t* tv_data_line(void);


#ifdef __cplusplus
};
#endif


#endif
