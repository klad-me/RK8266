#include <stdio.h>
#include <stdint.h>
#include <string.h>


uint8_t zkg[1024];


int main()
{
    // Читаем шрифт
    FILE *f=fopen("zkg.bin", "rb");
    fread(zkg, 1, sizeof(zkg), f);
    fclose(f);
    
    // Инвертируем шрифт
    for (int i=0; i<1024; i++)
	zkg[i]^=0xff;
    
    f=fopen("font.bin", "wb");
    fwrite(zkg, 1, sizeof(zkg), f);
    memset(zkg, 0x00, sizeof(zkg));
    fwrite(zkg, 1, sizeof(zkg), f);	// вторая часть пустая
    fclose(f);
}
