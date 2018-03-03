#ifndef I2S_H
#define I2S_H


#include <c_types.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef const uint8_t* (*i2s_cb_t)(void);


void i2s_init(i2s_cb_t cb, int size);
void i2s_start(void);


#ifdef __cplusplus
}
#endif


#endif

