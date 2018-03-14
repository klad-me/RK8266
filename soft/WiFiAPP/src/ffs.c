#include "ffs.h"

#ifdef __ets__
    #include "ets.h"
#else
    #include <osapi.h>
    #include <user_interface.h>
#endif
#include "crc8.h"
#include "str.h"


FILE fat[FAT_SIZE];
static uint8_t free_sect[128];	// максимум 4мб
static uint32_t f_size;



#ifdef __ets__
    typedef unsigned int	uint32;
    #define spi_flash_read(addr, buf, size)	SPIRead(addr, buf, size)
    #define spi_flash_write(addr, buf, size)	SPIWrite(addr, buf, size)
    #define spi_flash_erase_sector(addr)	SPIEraseSector(addr)
#endif


#define FFS_AT		0x80000


static void f_init(void)
{
    struct rom_header
    {
        uint8_t magic;
        uint8_t count;
        uint8_t flags1;
        uint8_t flags2;
    } hdr;
    
    // Читаем заголовок
    spi_flash_read(0, (uint32*)&hdr, sizeof(hdr));
    
    // Определяем размер флэша
    switch (hdr.flags2 >> 4)
    {
	case 3:
	    // 16 Мбит
	    f_size=2048*1024 - FFS_AT;
	    break;
	
	case 4:
	    // 32 Мбит
	    f_size=4096*1024 - FFS_AT;
	    break;
	
	case 2:
	default:
	    // 8 Мбит
	    f_size=1024*1024 - FFS_AT;
	    break;
    }
}


static void f_read(uint32_t pos, uint8_t *data, int size)
{
    spi_flash_read(FFS_AT+pos, (uint32*)data, size);
}


static void f_write(uint32_t pos, const uint8_t *data, int size)
{
    spi_flash_write(FFS_AT+pos, (const uint32*)data, size);
}


static void f_erase(uint32_t pos)
{
    spi_flash_erase_sector((FFS_AT+pos) / 4096);
}


static void ffs_cvt_name(const char *name, char *tmp)
{
    uint8_t i=0;
    
    while ( (i<8) && (name[i]) && (name[i]!='.') )
    {
	tmp[i]=to_upper(name[i]);
	i++;
    }
    
    while (i<8) tmp[i++]=0;
}


void ffs_init(void)
{
    uint16_t n;
    uint16_t n_free=0, n_removed=0;
    
    // Инитим флэш
    f_init();
    f_size-=sizeof(fat);	// убираем из размера размер FAT
    f_size/=4096;		// сразу в секторы 4к
    f_size&=~0x07;		// чтобы размер был кратным 8
    
    
    // Читаем FAT
    f_read(0, (uint8_t*)fat, sizeof(fat));
    
    // Проверяем таблицу FAT
    for (n=0; n<FAT_SIZE; n++)
    {
	bool is_free=true;
	uint8_t *data=(uint8_t*)&fat[n];
	uint8_t i;
	
	// Проверяем - может быть ячейка свободна
	for (i=0; i<sizeof(FILE); i++)
	    if (data[i] != 0xff)
	    {
		is_free=false;
		break;
	    }
	
	if (is_free)
	{
	    n_free++;
	    continue;
	}
	
	// Проверяем контрольную сумму и параметры файла
	if ( (CRC8(CRC8_INIT, data, sizeof(FILE)) != CRC8_OK) ||
	     (fat[n].page >= f_size) ||
	     (fat[n].page + ((fat[n].size + 4095) / 4096) > f_size) )
	{
	    // Ошибка - очищаем запись
	    fat[n].type=TYPE_REMOVED;
	    n_removed++;
	    continue;
	}
    }
    
    // Создаем таблицу свободных секторов
    ets_memset(free_sect, 0xff, f_size / 8);
    for (n=0; n<FAT_SIZE; n++)
    {
	// Пропускаем удаленные и пустые записи
	if ( (fat[n].type == TYPE_REMOVED) ||
	     (fat[n].type == TYPE_FREE) )
	    continue;
	
	uint16_t page=fat[n].page;
	int size=fat[n].size;
	while (size > 0)
	{
	    free_sect[page >> 3]&=~(1<<(page & 0x07));
	    page++;
	    size-=4096;
	}
    }
    
    // Если таблица слишком грязная - чистим ее и записываем на место
    if ( (n_free < 32) && (n_removed > 0) )
    {
	// Чистим таблицу
	for (n=0; n<FAT_SIZE; n++)
	{
	    if (fat[n].type==TYPE_REMOVED)
	    {
		ets_memset(&fat[n], 0xff, sizeof(FILE));
	    }
	}
	
	// Записываем
	f_erase(0);
	f_write(0, (uint8_t*)fat, sizeof(fat));
    }
}


uint32_t ffs_size(void)
{
    return f_size*4096;
}


uint32_t ffs_free(void)
{
    static const uint8_t n_bits[16] =
    {
	0, 1, 1, 2, 1, 2, 2, 3, 
	1, 2, 2, 3, 2, 3, 3, 4
    };
    uint32_t size=0;
    uint16_t i;
    for (i=0; i<f_size/8; i++)
    {
	size+=n_bits[free_sect[i] >> 4] + n_bits[free_sect[i] & 0x0f];
    }
    return size * 4096;
}


void ffs_read(uint16_t n, uint16_t offs, uint8_t *data, uint16_t size)
{
    f_read(sizeof(fat) + fat[n].page*4096 + offs, data, size);
}


int16_t ffs_create(const char *fname, uint8_t type, uint16_t size)
{
    uint16_t n;
    
    // Ищем свободную запись в FAT
    for (n=0; n<FAT_SIZE; n++)
    {
	if (fat[n].type==TYPE_FREE) break;
    }
    if (n>=FAT_SIZE) return -1;	// нет записей
    
    // Ищем свободное место
    uint16_t s_count=(size+4095)/4096;	// получаем кол-во секторов
    uint16_t s=0;
    while (s < f_size)
    {
	// Пропускаем занятые секторы
	while ( (s < f_size) && (! (free_sect[s >> 3] & (1<<(s & 0x07)))) ) s++;
	if (s >= f_size) break;
	
	// Считаем размер пустого блока
	uint16_t begin=s++;	// запоминаем начало пустого блока
	while ( (s < f_size) && (s-begin < s_count) && (free_sect[s >> 3] & (1<<(s & 0x07))) ) s++;
	if (s-begin < s_count) continue;	// блок слишком короткий
	
	// Подходящий блок найден
	s=begin;
	break;
    }
    if (s >= f_size) return -1;	// нет места
    
    // Заполняем запись FAT
    ffs_cvt_name(fname, fat[n].name);
    fat[n].page=s;
    fat[n].size=size;
    fat[n].type=type;
    fat[n].reserved=0;
    fat[n].crc8=CRC8(CRC8_INIT, (uint8_t*)&fat[n], sizeof(FILE)-1);
    
    // Записываем FAT
    f_write(n*sizeof(FILE), (uint8_t*)&fat[n], sizeof(FILE));
    
    // Отмечаем сектора как занятые
    while (s_count--)
    {
	free_sect[s >> 3]&=~(1<<(s & 0x07));
	s++;
    }
    
    return n;
}


void ffs_writeData(uint16_t n, uint16_t offs, const uint8_t *data, uint16_t size)
{
    // Записываем с предварительным стиранием
    while (size > 0)
    {
	// Получаем адрес
	uint32_t addr=sizeof(fat) + fat[n].page*4096 + offs;
	
	// Стираем страницу, если надо
	if ((addr & 4095)==0) f_erase(addr);
	
	// Получаем размер данных (он должен быть до конца страницы)
	int s=4096 - (addr & 4095);	// сколько осталось до конца страницы
	if (s > size) s=size;
	
	// Записываем
	f_write(addr, data, s);
	offs+=s;
	data+=s;
	size-=s;
    }
}


bool ffs_write(const char *fname, uint8_t type, const uint8_t *data, uint16_t size)
{
    // Создаем файл
    int16_t n=ffs_create(fname, type, size);
    if (n < 0) return false;
    
    // Записываем данные
    ffs_writeData(n, 0, data, size);
    
    return true;
}


int16_t ffs_find(const char *fname)
{
    uint16_t n;
    char tmp[8];
    
    ffs_cvt_name(fname, tmp);
    
    for (n=0; n<FAT_SIZE; n++)
    {
	if ( (fat[n].type == TYPE_REMOVED) || (fat[n].type == TYPE_FREE) ) continue;
	if (ets_memcmp(fat[n].name, tmp, 8) == 0) return n;
    }
    
    return -1;
}


uint32_t ffs_flash_addr(uint16_t n)
{
    return FFS_AT + sizeof(fat) + fat[n].page * 4096;
}


void ffs_remove(uint16_t n)
{
    // Помечаем запись как стертую
    fat[n].type=TYPE_REMOVED;
    
    // Записываем FAT
    f_write(n*sizeof(FILE), (uint8_t*)&fat[n], sizeof(FILE));
    
    // Очищаем занятые сектора
    uint16_t s_count=(fat[n].size+4095)/4096;	// получаем кол-во секторов
    uint16_t s=fat[n].page;
    while (s_count--)
    {
	free_sect[s >> 3]|=(1<<(s & 0x07));
	s++;
    }
}


const char* ffs_name(uint16_t n)
{
    static char tmp[9];
    ets_memcpy(tmp, fat[n].name, 8);
    tmp[8]=0;
    return tmp;
}
