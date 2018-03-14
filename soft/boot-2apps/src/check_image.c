#include "check_image.h"

#include "ets.h"


#define CHKSUM_INIT 0xef
#define SECTOR_SIZE 0x1000
#define ROM_MAGIC          0xe9
#define ROM_MAGIC_NEW1 0xea
#define ROM_MAGIC_NEW2 0x04
// buffer size, must be at least 0x10 (size of rom_header_new structure)
#define BUFFER_SIZE 0x100


typedef void usercode(void);

// standard rom header
typedef struct {
        // general rom header
        uint8_t magic;
        uint8_t count;
        uint8_t flags1;
        uint8_t flags2;
        usercode* entry;
} rom_header;

typedef struct {
        uint8_t* address;
        uint32_t length;
} section_header;

// new rom header (irom section first) there is
// another 8 byte header straight afterward the
// standard header
typedef struct {
        // general rom header
        uint8_t magic;
        uint8_t count; // second magic for new header
        uint8_t flags1;
        uint8_t flags2;
        uint32_t entry;
        // new type rom, lib header
        uint32_t add; // zero
        uint32_t len; // length of irom section
} rom_header_new;


uint32_t check_image(uint32_t readpos)
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t sectcount;
    uint8_t sectcurrent;
    uint8_t *writepos;
    uint8_t chksum = CHKSUM_INIT;
    uint32_t loop;
    uint32_t remaining;
    uint32_t romaddr;
    
    rom_header_new *header = (rom_header_new*)buffer;
    section_header *section = (section_header*)buffer;
    
    if (readpos == 0 || readpos == 0xffffffff)
    {
    	return 0;
    }
    
    // read rom header
    if (SPIRead(readpos, header, sizeof(rom_header_new)) != 0)
    {
    	return 0;
    }
    
    // check header type
    if (header->magic == ROM_MAGIC)
    {
	// old type, no extra header or irom section to skip over
	romaddr = readpos;
	readpos += sizeof(rom_header);
	sectcount = header->count;
    } else
    if (header->magic == ROM_MAGIC_NEW1 && header->count == ROM_MAGIC_NEW2)
    {
	// new type, has extra header and irom section first
	romaddr = readpos + header->len + sizeof(rom_header_new);
	// skip the extra header and irom section
	readpos = romaddr;
	// read the normal header that follows
	if (SPIRead(readpos, header, sizeof(rom_header)) != 0)
	{
	    return 0;
	}
	sectcount = header->count;
	readpos += sizeof(rom_header);
    } else
    {
    	return 0;
    }
    
    // test each section
    for (sectcurrent = 0; sectcurrent < sectcount; sectcurrent++)
    {
	// read section header
	if (SPIRead(readpos, section, sizeof(section_header)) != 0)
	{
	    return 0;
	}
	readpos += sizeof(section_header);
	
	// get section address and length
	writepos = section->address;
	remaining = section->length;
	
	while (remaining > 0)
	{
	    // work out how much to read, up to BUFFER_SIZE
	    uint32_t readlen = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
	    // read the block
	    if (SPIRead(readpos, buffer, readlen) != 0)
	    {
	    	return 0;
	    }
	    // increment next read and write positions
	    readpos += readlen;
	    writepos += readlen;
	    // decrement remaining count
	    remaining -= readlen;
	    // add to chksum
	    for (loop = 0; loop < readlen; loop++)
	    {
		chksum ^= buffer[loop];
	    }
	}
    }
    
    // round up to next 16 and get checksum
    readpos = readpos | 0x0f;
    if (SPIRead(readpos, buffer, 1) != 0)
    {
    	return 0;
    }
    
    // compare calculated and stored checksums
    if (buffer[0] != chksum)
    {
	return 0;
    }
    
    return romaddr;
}
