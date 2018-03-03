#ifndef LIBESP_GPIO16_H
#define LIBESP_GPIO16_H

#include <os_type.h>


#ifdef __cplusplus
extern "C" {
#endif


void gpio16_output_conf(void);
void gpio16_output_set(uint8 value);
void gpio16_input_conf(void);
uint8 gpio16_input_get(void);


#ifdef __cplusplus
}
#endif


#endif
