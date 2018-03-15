#include "files.h"

#include "ets.h"
#include "ffs.h"
#include "i8080_hal.h"


#define DEBUG(...)	do { ets_printf(__VA_ARGS__); ets_delay_us(1000000); } while(0)


int16_t load_file(uint16_t n)
{
    uint8_t hdr[8];
    uint8_t hdr_size;
    uint16_t start, end;
    
    // Читаем заголовок
    ffs_read(n, 0, hdr, sizeof(hdr));
    
    // Проверим байт 0xE6
    if (hdr[0]==0xE6)
    {
	// Есть байт - пропускаем его
	start=(hdr[1] << 8) | hdr[2];
	end=(hdr[3] << 8) | hdr[4];
	hdr_size=5;
    } else
    {
	start=(hdr[0] << 8) | hdr[1];
	end=(hdr[2] << 8) | hdr[3];
	hdr_size=4;
    }
    
    uint16_t size=end-start+1;
    
    // Проверим адреса
    if ( (end < start) ||
	 (start >= 0x8000) ||
	 (start+size > 0x8000) )
    {
	// Неверный адрес в памяти
	return -1;
    }
    
    // Копируем в память
    uint8_t *data=i8080_hal_memory()+start;
    ets_memcpy(data, hdr+hdr_size, sizeof(hdr)-hdr_size);
    uint16_t offs=8;
    data+=sizeof(hdr)-hdr_size;
    size-=sizeof(hdr)-hdr_size;
    while (size > 0)
    {
	// Читать надо с выравниванием по 4 байта
	uint32_t tmp[128];
	uint16_t s=(size > sizeof(tmp)) ? sizeof(tmp) : size;
	ffs_read(n, offs, (uint8_t*)tmp, (s+3) & ~0x03);
	ets_memcpy(data, tmp, s);
	data+=s;
	offs+=s;
	size-=s;
    }
    
    // Готово
    return start;
}
