#include <stdio.h>
#include <stdint.h>


uint8_t zkg[1024];
uint8_t z0[1024], z1[1024], z2[1024], z3[1024];


void dump(uint8_t *z, const char *name)
{
    printf("static const uint8_t %s[1024]=\n{", name);
    for (int i=0; i<1024; i++)
    {
	if ((i%128)==0) printf("\n    // Line %d", i/128);
	if ((i%16)==0) printf("\n    ");
	printf("0x%02x, ", z[i]);
    }
    printf("\n};\n\n");
}


int main()
{
    // Читаем шрифт
    FILE *f=fopen("zkg.bin", "rb");
    fread(zkg, 1, sizeof(zkg), f);
    fclose(f);
    
    // Инвертируем шрифт
    for (int i=0; i<1024; i++)
	zkg[i]^=0xff;
    
    // Цикл по символам
    for (int n=0; n<128; n++)
    {
	// Цикл по строкам
	for (int l=0; l<8; l++)
	{
	    uint8_t b=zkg[n*8+l];
	    
	    // Нормальный буфер, но с другой адресацией
	    z0[l*128+n]=b;
	    
	    // Первый буфер 6+2
	    z1[l*128+n]=((b<<2) & 0xFC) | ((b>>4) & 0x03);
	    
	    // Второй буфер 4+4
	    z2[l*128+n]=((b<<4) & 0xF0) | ((b>>2) & 0x0F);
	    
	    // Третий буфер 2+6
	    z3[l*128+n]=((b<<6) & 0xC0) | (b & 0x3F);
	}
    }
    
    
    // Создаем массивы
    /*dump(z1, "Z1");
    dump(z2, "Z2");
    dump(z3, "Z3");*/
    dump(z0, "zkg");
}
