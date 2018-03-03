#ifndef LIBESP_GPIO_LIB_H
#define LIBESP_GPIO_LIB_H


#include <c_types.h>


#ifdef __cplusplus
extern "C" {
#endif


#define GPIO_INV	0x20


extern const uint32_t gpio_mux[];


void gpio_init_input(uint8_t gpio);
void gpio_init_input_pu(uint8_t gpio);
void gpio_init_output(uint8_t gpio);
void gpio_on(uint8_t gpio);
void gpio_off(uint8_t gpio);
bool gpio_in(uint8_t gpio);


#ifdef __cplusplus
}
#endif


#endif
