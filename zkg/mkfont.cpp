#include <stdio.h>
#include <stdint.h>


uint8_t zkg[2048];
uint8_t z[8*128];
uint8_t g[8*16];


int main()
{
    // Читаем шрифт
    FILE *f=fopen("font.bin", "rb");
    fread(zkg, 1, sizeof(zkg), f);
    fclose(f);
    
    // Основной знакогенератор
    for (int n=0; n<128; n++)
    {
	// Цикл по строкам
	for (int l=0; l<8; l++)
	{
	    uint8_t b=zkg[n*8+l];
	    
	    // Делаем более удобную адресацию
	    z[l*128+n]=b;
	}
    }
    
    // Псевдографика
    for (int n=0; n<16; n++)
    {
	// Цикл по строкам
	for (int l=0; l<8; l++)
	{
	    uint8_t b=zkg[(128+n)*8+l];
	    
	    // Делаем более удобную адресацию
	    g[l*16+n]=b;
	}
    }
    
    // Создаем массив
    printf("#include \"zkg.h\"\n\n#include \"ets.h\"\n\nuint8_t zkg[1024];\n\nconst uint8_t AT_IRAM zkg_rom[1024]=\n{");
    for (int i=0; i<1024; i++)
    {
	if ((i%128)==0) printf("\n    // Line %d", i/128);
	if ((i%16)==0) printf("\n    ");
	printf("0x%02x, ", z[i]);
    }
    printf("\n};\n\nconst uint8_t AT_IRAM zkg_graph[128]=\n{");
    for (int i=0; i<128; i++)
    {
	if ((i%16)==0) printf("\n    // Line %d", i/16);
	if ((i%16)==0) printf("\n    ");
	printf("0x%02x, ", g[i]);
    }
    printf("\n};\n");
}
