#ifndef FFS_H
#define FFS_H


#ifdef __ets__
    #include "ets.h"
#else
    #include <c_types.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


enum
{
    TYPE_REMOVED=0,
    
    TYPE_TAPE,
    TYPE_PROG,
    TYPE_GAME,
    TYPE_UTIL,
    
    TYPE_FREE=0xff,
};


typedef struct FILE
{
    uint16_t page;
    uint16_t size;
    char     name[9];
    uint8_t  type;
    uint8_t  reserved;
    uint8_t  crc8;
} FILE;

#define FAT_SIZE	(4096/sizeof(FILE))
extern FILE fat[FAT_SIZE];


void ffs_init(void);

uint32_t ffs_size(void);
uint32_t ffs_free(void);

void ffs_read(uint16_t n, uint16_t offs, uint8_t *data, uint16_t size);
bool ffs_write(const char *fname, uint8_t type, const uint8_t *data, uint16_t size);

int16_t ffs_create(const char *fname, uint8_t type, uint16_t size);
void ffs_writeData(uint16_t n, uint16_t offs, const uint8_t *data, uint16_t size);

int16_t ffs_find(const char *fname);
uint32_t ffs_flash_addr(uint16_t n);

void ffs_remove(uint16_t n);
int16_t ffs_rename(uint16_t n, const char *fname);

const char* ffs_name(uint16_t n);	// в статической переменной


#ifdef __cplusplus
}
#endif


#endif
