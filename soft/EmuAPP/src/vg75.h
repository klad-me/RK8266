#ifndef VG75_H
#define VG75_H


#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


struct screen
{
    uint8_t screen_w, screen_h, underline_y, char_h, attr_visible;
    uint8_t x_offset, y_offset;
    uint8_t cursor_x, cursor_y, cursor_type;
    uint8_t dma_burst;
    uint8_t *vram;
    char overlay[64];
    uint16_t overlay_timer;
};
extern struct screen screen;


void vg75_init(uint8_t *vram);

void vg75_overlay(const char *str);

void vg75_W(uint8_t A, uint8_t value);
uint8_t vg75_R(uint8_t A);

void ik57_W(uint8_t A, uint8_t value);
uint8_t ik57_R(uint8_t A);


#ifdef __cplusplus
};
#endif


#endif
