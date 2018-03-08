#include "ets.h"
#include "check_image.h"
#include "ram_loader.h"


uint32_t main_program(void)
{
    uint32_t runAddr;
    
    // Отключаем кэш, это дает +32кб IRAM
    CLEAR_PERI_REG_MASK(0x3ff00024, 0x00000018);
    
    // Настраиваем UART
    uart_div_modify(0, 450);
    
    ets_printf("\r\nFastBoot IRAM64k\r\n");
    
    // Check rom is valid
    runAddr=check_image(0x1000);
    
    if (runAddr==0)
    {
	ets_printf("No good rom available.\r\n");
	return 0;
    }
    
    ets_printf("Booting rom.\r\n");
    // copy the loader to top of iram
    ets_memcpy((void*)_text_addr, _text_data, _text_len);
    // return address to load from
    return runAddr;

}


// assembler stub uses no stack space
// works with gcc
void call_user_start(void) {
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 main_program\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ?success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
}
