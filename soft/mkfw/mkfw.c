#include <stdio.h>
#include <inttypes.h>
#include <string.h>


uint16_t CRC16(uint16_t crc, const uint8_t *data, uint16_t size)
{
    uint16_t i;
    uint8_t x,cnt;
    
    for (i=0; i<size; i++)
    {
        x=data[i];
        for (cnt=0; cnt<8; cnt++)
        {
            if ((x^crc)&1) crc=((crc^0x4002)>>1)|0x8000; else
                           crc=crc>>1;
            x>>=1;
        }
    }

    return crc;
}


uint8_t data[0x7C000];

struct header
{
    char magic[8];
    uint16_t size4k;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t crc;
};
struct header *hdr=(struct header*)data;
uint16_t *crcdata=(uint16_t*)(data + sizeof(struct header));


int read(uint8_t *buf, const char *fname, int max_size)
{
    FILE *f=fopen(fname, "rb");
    if (!f)
    {
	perror(fname);
	return 0;
    }
    fread(buf, 1, max_size, f);
    fclose(f);
    return 1;
}


#define FW	"0x00000.bin"
#define UPDATE	"fota.bin"


int main()
{
    FILE *f;
    
    // Очищаем буфер
    memset(data, 0x00, sizeof(data));
    
    // Читаем загрузчик
    if (! read(data+0x00000, "../boot-2apps/out/boot.bin", 0x1000)) return -1;
    
    // Читаем прошивку эмулятора
    if (! read(data+0x01000, "../EmuAPP/out/emu-0x00000.bin", 0xF000)) return -1;
    
    // Читаем прошивку WiFi
    if (! read(data+0x10000, "../WiFiAPP/out/wifi.1.bin", 0x60000)) return -1;
    
    // Читаем справку
    if (! read(data+0x50000, "../help/help.bin", 0x10000)) return -1;
    
    // Читаем файловую систему HTTP
    if (! read(data+0x70000, "../WiFiAPP/httpfs/httpfs.bin", 0xC000)) return -1;
    
    
    // Сохраняем прошивку для программатора
    f=fopen(FW, "wb");
    if (!f)
    {
	perror(FW);
	return -1;
    }
    fwrite(data, 1, sizeof(data), f);
    fclose(f);
    
    
    // Заполняем заголовок
    memset(data, 0x00, 0x1000);	// вместо загрузчика будет заголовок для обновления
    memcpy(hdr->magic, "FWUPDATE", 8);
    hdr->size4k=0x7B;
    hdr->reserved1=0;
    hdr->reserved2=0;
    hdr->crc=CRC16(0xFFFF, (uint8_t*)hdr, sizeof(struct header)-2);
    
    // Заполняем контрольные суммы блоков
    int i;
    for (i=0; i<hdr->size4k; i++)
    {
	crcdata[i]=CRC16(0xFFFF, data+0x1000+i*0x1000, 0x1000);
    }
    
    // Сохраняем обновление
    f=fopen(UPDATE, "wb");
    if (!f)
    {
	perror(UPDATE);
	return -1;
    }
    fwrite(data, 1, sizeof(data), f);
    fclose(f);
    
    return 0;
}
