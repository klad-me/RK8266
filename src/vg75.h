#ifndef VG75_H
#define VG75_H


#include <os_type.h>


#ifdef __cplusplus
extern "C" {
#endif


void vg75_init(void);
void vg75_W(uint8_t A, uint8_t value);
uint8_t vg75_R(uint8_t A);
void ik57_W(uint8_t A, uint8_t value);
uint8_t ik57_R(uint8_t A);
void vg75_update(void);


#ifdef __cplusplus
};
#endif


#endif
