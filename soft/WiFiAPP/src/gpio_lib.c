#include "gpio_lib.h"

#include <osapi.h>
#include <user_interface.h>
#include <gpio.h>
#include "gpio16.h"


const uint32_t gpio_mux[] =
{
    PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0,
    PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1,
    PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2,
    PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3,
    PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,
    PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12,
    PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13,
    PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14,
    PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15,
};


void RAMFUNC gpio_init_input(uint8_t gpio)
{
    gpio&=0x1f;
    if (gpio < 16)
    {
	PIN_FUNC_SELECT(gpio_mux[gpio*2+0], gpio_mux[gpio*2+1]);
	gpio_output_set(0, 0, 0, (1 << gpio));
    } else
    {
	gpio16_input_conf();
    }
}


void RAMFUNC gpio_init_input_pu(uint8_t gpio)
{
    gpio&=0x1f;
    if (gpio < 16)
    {
	PIN_FUNC_SELECT(gpio_mux[gpio*2+0], gpio_mux[gpio*2+1]);
	gpio_output_set(0, 0, 0, (1 << gpio));
	PIN_PULLUP_EN(gpio_mux[gpio*2+0]);
    } else
    {
	gpio16_input_conf();
    }
}


void RAMFUNC gpio_init_output(uint8_t gpio)
{
    gpio&=0x1f;
    if (gpio<16)
    {
	PIN_FUNC_SELECT(gpio_mux[gpio*2+0], gpio_mux[gpio*2+1]);
	gpio_output_set(0, 0, (1 << gpio), 0);
    } else
    {
	gpio16_output_conf();
    }
}


void RAMFUNC gpio_on(uint8_t gpio)
{
    uint8_t n=(gpio & 0x1f);
    if (n < 16)
    {
	if (gpio & GPIO_INV)
	    gpio_output_set(0, (1 << gpio), 0, 0); else
	    gpio_output_set((1 << gpio), 0, 0, 0);
    } else
    {
	if (gpio & GPIO_INV)
	    gpio16_output_set(0); else
	    gpio16_output_set(1);
    }
}


void RAMFUNC gpio_off(uint8_t gpio)
{
    uint8_t n=(gpio & 0x1f);
    if (n < 16)
    {
	if (gpio & GPIO_INV)
	    gpio_output_set((1 << gpio), 0, 0, 0); else
	    gpio_output_set(0, (1 << gpio), 0, 0);
    } else
    {
	if (gpio & GPIO_INV)
	    gpio16_output_set(1); else
	    gpio16_output_set(0);
    }
}


bool RAMFUNC gpio_in(uint8_t gpio)
{
    uint8_t n=(gpio & 0x1f);
    if (n < 16)
    {
	if (gpio & GPIO_INV)
	    return (gpio_input_get() & (1<<n))==0; else
	    return (gpio_input_get() & (1<<n))!=0;
    } else
    {
	if (gpio & GPIO_INV)
	    return gpio16_input_get()==0;
	    return gpio16_input_get()!=0;
    }
}
