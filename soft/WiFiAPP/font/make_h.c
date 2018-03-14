#include <stdio.h>


int main(int argc,char **argv)
{
    FILE *f;
    int i,j,x;
    unsigned char font[256*12];
    unsigned char map[128];
    
    f=fopen("8x12.fnt","rb");
    fread(font,sizeof(font),1,f);
    fclose(f);
    
    f=fopen("koi.bin", "rb");
    fread(map,128,1,f);
    fclose(f);
    
    printf("#ifndef FONT8X12_H\n#define FONT8X12_H\n\n\nconst uint8_t font_8x12[3072] =\n{\n");
    for (i=0; i<12; i++)
    {
	printf("    // Line %d", i);
	for (j=0; j<256; j++)
	{
	    unsigned char c=j;
	    if (c >= 128) c=map[c-128];
	    if (j==128) fprintf(stderr, "c=%d  idx=%03X  %02X\n", c, c*12+i, font[c*12+i]);
	    if ((j % 16)==0) printf("\n    ");
	    printf("0x%02x,", font[c*12+i]);
	}
	printf("\n");
    }
    printf("};\n\n#endif\n");
    
    return 0;
}
