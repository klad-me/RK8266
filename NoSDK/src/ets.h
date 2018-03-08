#ifndef ETS_H
#define ETS_H


#include <inttypes.h>

typedef uint8_t bool;
#define false	0
#define true	1
#define FALSE	0
#define TRUE	1


#ifndef NULL
    #define NULL	((void*)0)
#endif


typedef enum {
    GPIO_PIN_INTR_DISABLE = 0,
    GPIO_PIN_INTR_POSEDGE = 1,
    GPIO_PIN_INTR_NEGEDGE = 2,
    GPIO_PIN_INTR_ANYEDGE = 3,
    GPIO_PIN_INTR_LOLEVEL = 4,
    GPIO_PIN_INTR_HILEVEL = 5
} GPIO_INT_TYPE;


extern void uart_div_modify(int no, unsigned int freq);

extern void gpio_init(void);
extern void gpio_output_set(uint32_t set_mask,
                     uint32_t clear_mask,
                     uint32_t enable_mask,
                     uint32_t disable_mask);
extern uint32_t gpio_input_get(void);
extern void gpio_pin_intr_state_set(uint32_t i, GPIO_INT_TYPE intr_state);

extern uint32_t SPIRead(uint32_t addr, void *outptr, uint32_t len);
extern uint32_t SPIEraseSector(int);
extern uint32_t SPIWrite(uint32_t addr, void *inptr, uint32_t len);

extern void ets_printf(char*, ...);
extern void ets_install_putc1(void *routine);

extern void ets_delay_us(int);

extern void ets_memset(void*, uint8_t, uint32_t);
extern void ets_memcpy(void*, const void*, uint32_t);
extern int ets_memcmp(const void *s1, const void *s2, uint32_t n);

int ets_sprintf(char *str, const char *format, ...)  __attribute__ ((format (printf, 2, 3)));
extern int ets_strcmp(const char *s1, const char *s2);
extern char *ets_strcpy(char *dest, const char *src);
extern int ets_strlen(const char *s);
extern int ets_strncmp(const char *s1, const char *s2, int len);
extern char *ets_strncpy(char *dest, const char *src, int n);
extern char *ets_strstr(const char *haystack, const char *needle);
extern char *ets_strcat(char *dest, const char *src);
extern char *ets_strchr(const char *s, int c);

extern int ets_get_cpu_frequency(void);
extern void ets_update_cpu_frequency(int freqmhz);

extern void ets_isr_attach(int intr, void *handler, void *arg);
extern void ets_isr_mask(unsigned intr);
extern void ets_isr_unmask(unsigned intr);

extern void rom_i2c_writeReg(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t data);
extern void rom_i2c_writeReg_Mask(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t Msb, uint32_t Lsb, uint32_t indata);
extern uint8_t rom_i2c_readReg_Mask(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t Msb, uint32_t Lsb);
extern uint8_t rom_i2c_readReg(uint32_t block, uint32_t host_id, uint32_t reg_add);

extern void software_reset(int param);


#define BIT(nr)			(1UL << (nr))

#define BIT31	0x80000000
#define BIT30	0x40000000
#define BIT29	0x20000000
#define BIT28	0x10000000
#define BIT27	0x08000000
#define BIT26	0x04000000
#define BIT25	0x02000000
#define BIT24	0x01000000
#define BIT23	0x00800000
#define BIT22	0x00400000
#define BIT21	0x00200000
#define BIT20	0x00100000
#define BIT19	0x00080000
#define BIT18	0x00040000
#define BIT17	0x00020000
#define BIT16	0x00010000
#define BIT15	0x00008000
#define BIT14	0x00004000
#define BIT13	0x00002000
#define BIT12	0x00001000
#define BIT11	0x00000800
#define BIT10	0x00000400
#define BIT9	0x00000200
#define BIT8	0x00000100
#define BIT7	0x00000080
#define BIT6	0x00000040
#define BIT5	0x00000020
#define BIT4	0x00000010
#define BIT3	0x00000008
#define BIT2	0x00000004
#define BIT1	0x00000002
#define BIT0	0x00000001


#define REG_SET_BIT(_r, _b)	(*(volatile uint32_t*)(_r) |= (_b))
#define REG_CLR_BIT(_r, _b)	(*(volatile uint32_t*)(_r) &= ~(_b))


#define ETS_UNCACHED_ADDR(addr)	(addr)
#define READ_PERI_REG(addr) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_PERI_REG(addr, val) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr))) = (uint32_t)(val)
#define CLEAR_PERI_REG_MASK(reg, mask) WRITE_PERI_REG((reg), (READ_PERI_REG(reg)&(~(mask))))
#define SET_PERI_REG_MASK(reg, mask)   WRITE_PERI_REG((reg), (READ_PERI_REG(reg)|(mask)))
#define GET_PERI_REG_BITS(reg, hipos,lowpos)      ((READ_PERI_REG(reg)>>(lowpos))&((1<<((hipos)-(lowpos)+1))-1))
#define SET_PERI_REG_BITS(reg,bit_map,value,shift) (WRITE_PERI_REG((reg),(READ_PERI_REG(reg)&(~((bit_map)<<(shift))))|((value)<<(shift)) ))


#define ETS_INTR_ENABLE(inum) \
    ets_isr_unmask((1<<inum))

#define ETS_INTR_DISABLE(inum) \
    ets_isr_mask((1<<inum))

#define ETS_GPIO_INTR_ATTACH(func, arg) \
    ets_isr_attach(ETS_GPIO_INUM, (func), (void *)(arg))

#define ETS_GPIO_INTR_ENABLE() \
    ETS_INTR_ENABLE(ETS_GPIO_INUM)


#define PERIPHS_GPIO_BASEADDR		0x60000300
#define GPIO_REG_READ(reg)		READ_PERI_REG(PERIPHS_GPIO_BASEADDR + reg)
#define GPIO_REG_WRITE(reg, val)	WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + reg, val)
#define GPIO_STATUS_ADDRESS		0x1c
#define GPIO_STATUS_W1TC_ADDRESS	0x24
#define ETS_GPIO_INUM			4


#endif
