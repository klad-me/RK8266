#include "reboot.h"

#include "ets.h"


void reboot(uint32_t value)
{
    // Значение для загрузчика
    WRITE_PERI_REG(0x60001200, value);
    
    // Запускаем Watchdog, чтобы он нас перегрузил
    WRITE_PERI_REG(0x60000900, 0x3C);	// wdt disable, stage1 disable
    WRITE_PERI_REG(0x60000904, 1);		// reload stage0 = 0,84sec
    WRITE_PERI_REG(0x60000900, 0x3D);	// wdt enable, stage1 disable
    
    while(1);
}
