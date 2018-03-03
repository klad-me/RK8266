#ifndef I8080_HAL_H
#define I8080_HAL_H


#ifdef __cplusplus
extern "C" {
#endif


extern int i8080_hal_memory_read_word(int addr);
extern void i8080_hal_memory_write_word(int addr, int word);

extern int i8080_hal_memory_read_byte(int addr);
extern void i8080_hal_memory_write_byte(int addr, int byte);

extern int i8080_hal_io_input(int port);
extern void i8080_hal_io_output(int port, int value);

extern void i8080_hal_iff(int on);

extern unsigned char* i8080_hal_memory(void);

void i8080_hal_init(void);


#ifdef __cplusplus
};
#endif


#endif
