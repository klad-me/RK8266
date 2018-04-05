#include "Web.h"

#include <osapi.h>
#include "ffs.h"
#include "str.h"


extern char __BUILD_NUMBER__;


struct map
{
    const char *path;
    uint8_t type;
};


static struct map map[]=
    {
	"/tape/",	TYPE_TAPE,
	"/prog/",	TYPE_PROG,
	"/game/",	TYPE_GAME,
	"/util/",	TYPE_UTIL,
	0,		0
    };


uint32_t Web::webPutStart(const char *path, int size)
{
    uint8_t type=0;
    
    for (uint8_t i=0; map[i].path; i++)
    {
	if (! os_strncmp(path, map[i].path, os_strlen(map[i].path)))
	{
	    // Нашли
	    type=map[i].type;
	    path+=os_strlen(map[i].path);
	    break;
	}
    }
    if (! type) return 0;
    
    // Проверим, вдруг файл уже существует
    if (ffs_find(path) >= 0) return 0;
    
    // Создаем файл
    int16_t n=ffs_create(path, type, size);
    if (n<0) return 0;
    
    // Возвращаем адрес во Flash
    return ffs_flash_addr(n) | 0x80000000;
}


void Web::webPutData(const char *path, const uint8_t *data, int size)
{
}


const char* Web::webPutEnd(const char *path)
{
    return 0;
}


const char* Web::webGet(const char *path, uint32_t *dataPtr, int *size)
{
    // Спец-пути
    if (! os_strcmp(path, "/dir.js"))
    {
	return getDir(dataPtr, size);
    } else
    if (! os_strncmp(path, "/del/", 5))
    {
	return delFile(path+5, dataPtr, size);
    }
    
    uint8_t type=0;
    
    for (uint8_t i=0; map[i].path; i++)
    {
	if (! os_strncmp(path, map[i].path, os_strlen(map[i].path)))
	{
	    // Нашли
	    type=map[i].type;
	    path+=os_strlen(map[i].path);
	    break;
	}
    }
    if (! type) return 0;
    
    return getFile(type, path, dataPtr, size);
}


const char* Web::getDir(uint32_t *dataPtr, int *size)
{
    // Код будет следующим: addFile(0x00,"",0x0000);\n (len=25)
    os_printf("getdir\n");
    
    // Считаем размер кода
    uint16_t dataSize=0;
    for (uint16_t n=0; n<FAT_SIZE; n++)
    {
	if ( (fat[n].type != TYPE_REMOVED) &&
	     (fat[n].type != TYPE_FREE) )
	    dataSize+=25+os_strlen(ffs_name(n));
    }
    
    // Информация о размере флэша: flashInfo(0x00000000, 0x00000000);\n (len=35)
    dataSize+=35;
    
    // Информация о версии прошивки: fwInfo(0x0000);\n (len=16)
    dataSize+=16;
    
    // Создаем данные
    char *data=new char[dataSize+8];
    if (!data) return 0;
    
    // Заполняем
    char *ss=data;
    for (uint16_t n=0; n<FAT_SIZE; n++)
    {
	if ( (fat[n].type != TYPE_REMOVED) &&
	     (fat[n].type != TYPE_FREE) )
	{
	    ss+=os_sprintf(ss, "addFile(0x%02X,\"%s\",0x%04X);\n", fat[n].type, ffs_name(n), fat[n].size);
	}
    }
    
    ss+=os_sprintf(ss, "flashInfo(0x%08X, 0x%08X);\n", (unsigned int)ffs_size(), (unsigned int)ffs_free());
    ss+=os_sprintf(ss, "fwInfo(0x%04X);\n", (int)&__BUILD_NUMBER__);
    
    (*dataPtr)=(uint32_t)data;
    (*size)=dataSize;
    
    os_printf("getdir ok\n");
    return os_strdup("HTTP/1.0 200 Ok\r\nConnection: close\r\nContent-Type: application/javascript; charset=utf-8\r\n\r\n");
}


const char* Web::getFile(uint8_t type, const char *path, uint32_t *dataPtr, int *size)
{
    os_printf("GET '%s' type=%d\n", path, type);
    
    int16_t n=ffs_find(path);
    os_printf("  find %d\n", n);
    if (n < 0) return 0;
    os_printf("  file type %d\n", fat[n].type);
    if (fat[n].type != type) return 0;
    
    (*dataPtr)=ffs_flash_addr(n) | 0x80000000;
    (*size)=fat[n].size;
    
    char hdr[128];
    os_sprintf(hdr, "HTTP/1.0 200 Ok\r\nConnection: close\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n", (*size));
    
    return os_strdup(hdr);
}


const char* Web::delFile(const char *path, uint32_t *dataPtr, int *size)
{
    os_printf("DEL '%s'\n", path);
    
    int16_t n=ffs_find(path);
    if (n < 0) return 0;
    
    ffs_remove(n);
    
    (*dataPtr)=0;
    (*size)=0;
    
    return os_strdup("HTTP/1.0 302 Found\r\nConnection: close\r\nLocation: /\r\n\r\n");
}
