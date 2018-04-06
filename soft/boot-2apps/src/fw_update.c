#include "fw_update.h"

#include "ets.h"
#include "crc16.h"
#include "reboot.h"


struct header
{
    char magic[8];
    uint16_t size4k;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t crc;
};

static uint8_t hdrbuf[4096];
static struct header *hdr=(struct header*)hdrbuf;
static uint16_t *crcdata=(uint16_t*)(hdrbuf + sizeof(struct header));
static uint8_t buf[4096];


uint8_t check_fw_crc(uint32_t addr)
{
    int i;
    for (i=0; i<hdr->size4k; i++)
    {
	ets_printf(" %02X", addr/4096 + i);
	SPIRead(addr+i*4096, buf, 4096);
	if (CRC16(CRC16_INIT, buf, 4096) != crcdata[i])
	{
	    ets_printf("\nFW Update: CRC error for sector %02X\n", addr/4096 + i);
	    return 0;
	}
    }
    ets_printf("\n");
    return 1;
}


void fw_update(void)
{
    int i;
    
    // Читаем и проверяем заголовок
    SPIRead(0x80000, &hdrbuf, 4096);
    if ( (ets_memcmp(hdr->magic, "FWUPDATE", sizeof(hdr->magic))!=0) ||
	 (CRC16(CRC16_INIT, hdrbuf, sizeof(struct header)) != CRC16_OK) )
    {
	ets_printf("FW Update not found\n");
	return;
    }
    
    // На первом проходе проверяем контрольные суммы данных во флэше
    ets_printf("FW Update: checking firmware...");
    if (! check_fw_crc(0x81000))
    {
	// Не будем обновлять
	return;
    }
    
    // Копируем данные
    ets_printf("FW Update: flashing...");
    for (i=0; i<hdr->size4k; i++)
    {
	ets_printf(" %02X", 1+i);
	SPIRead(0x81000+i*4096, buf, 4096);
	SPIUnlock();
	SPIEraseSector(0x1+i);
	SPIUnlock();
	SPIWrite(0x01000+i*4096, buf, 4096);
    }
    ets_printf("\n");
    
    // Проверяем прошивку
    ets_printf("Checking firmware...");
    if (! check_fw_crc(0x01000))
    {
	// Не будем стирать прошивку
	ets_printf("FW Update: updated firmware ERROR ! - not erasing update\n");
	reboot(0);
    }
    
    // Можно стирать заголовок обновления
    SPIUnlock();
    SPIEraseSector(0x80);
    
    // Готово
    ets_printf("FW Update: upgrade successfull !\n");
    
    reboot(0);
}
