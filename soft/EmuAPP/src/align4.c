#include "align4.h"

#include "ets.h"


uint8_t r_u8(const uint8_t *addr)
{
    volatile uint32_t bytes = *(volatile uint32_t*)((uint32_t)addr & ~3);
    return ((volatile uint8_t*)&bytes)[(uint32_t)addr & 3];
}


uint16_t r_u16(const uint16_t *addr)
{
    volatile uint32_t bytes = *(volatile uint32_t*)((uint32_t)addr & ~3);
    return ((volatile uint16_t*)&bytes)[((uint32_t)addr >> 1) & 1];
}
