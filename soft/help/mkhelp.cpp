#include <stdio.h>
#include <stdint.h>
#include <string.h>


// Таблица перекодировки из KOI8-R
const uint8_t xlat[] =
{
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x21, 0x22, 0x23,
  0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
  0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53,
  0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x20,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
  0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x65, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x65,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
  0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x78, 0x60, 0x61, 0x62, 0x63,
  0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
  0x7c, 0x7d, 0x7e, 0x78
};


#define HELP_FILE	"help.txt"
#define HELP_BIN	"help.bin"


// Аттрибуты текста
#define BLINK           0x02
#define REVERSE         0x10
#define UNDERLINE       0x20


typedef struct
{
    uint8_t text[64];
    uint8_t attr[64];
    
    uint8_t enc[73];
} LINE;

#define MAX_LINES	10240
LINE lines[MAX_LINES];
int n_lines;


int main()
{
    memset(lines, 0x00, sizeof(lines));
    n_lines=0;
    
    // Читаем и разбираем файл на экранные строки
    FILE *f=fopen(HELP_FILE, "r");
    if (!f)
    {
	perror(HELP_FILE);
	return -1;
    }
    char str[256];
    while (fgets(str, sizeof(str), f))
    {
	uint8_t attr=0;
	int x=0;
	bool tag=false;
	for (char *ss=str; (*ss) && ((*ss)!='\r') && ((*ss)!='\n'); ss++)
	{
	    if (tag)
	    {
		// Был символ тэга - дальше идет команда
		tag=false;
		switch (*ss)
		{
		    case '~':
			// Сброс аттрибутов
			attr=0;
			break;
		    
		    case 'u':
			// Подчеркивание
			attr^=UNDERLINE;
			break;
		    
		    case 'r':
			// Инверсия
			attr^=REVERSE;
			break;
		    
		    case 'b':
			// Мигание
			attr^=BLINK;
			break;
		    
		    default:
			// Ошибка
			fprintf(stderr, "Line %d: incorrect attribute '%c'\n", n_lines+1, (*ss));
			return -1;
		}
	    } else
	    {
		if ((*ss)=='~')
		{
		    // Начало команды
		    tag=true;
		} else
		{
		    // Символ
		    if (x >= 64)
		    {
			fprintf(stderr, "Line %d too long\n", n_lines+1);
			return -1;
		    }
		    if ((*ss)=='\t')
		    {
			// Табуляция
			do
			{
			    if (x >= 64)
			    {
				fprintf(stderr, "Line %d too long\n", n_lines+1);
				return -1;
			    }
			    lines[n_lines].text[x]=' ';
			    lines[n_lines].attr[x]=attr;
			    x++;
			} while ((x & 0x07)!=0);
		    } else
		    {
			// Просто символ
			lines[n_lines].text[x]=xlat[(uint8_t)(*ss)];
			lines[n_lines].attr[x]=attr;
			x++;
		    }
		}
	    }
	}
	n_lines++;
    }
    fclose(f);
    
    
    // Преобразовываем таблицы в псевдографику
#define LEFT	1
#define RIGHT	2
#define TOP	4
#define BOTTOM	8
    static const uint8_t pg[16]=
    {
	0x20,	// 0 -
	0xE0,	// 1 LEFT
	0xE0,	// 2 RIGHT
	0xE0,	// 3 LEFT+RIGHT = HORIZ
	0xE4,	// 4 TOP
	0xCC,	// 5 TOP+LEFT
	0xC8,	// 6 TOP+RIGHT
	0xDC,	// 7 TOP+LEFT+RIGHT
	0xE4,	// 8 BOTTOM
	0xC4,	// 9 BOTTOM+LEFT
	0xC0,	// A BOTTOM+RIGHT
	0xD0,	// B BOTTOM+LEFT+RIGHT
	0xE4,	// C BOTTOM+TOP = VERT
	0xD4,	// D BOTTOM+TOP+LEFT
	0xD8,	// E BOTTOM+TOP+RIGHT
	0xE8,	// F BOTTOM+TOP+RIGHT+LEFT = CROSS
    };
    for (int i=0; i<n_lines; i++)
    {
	char *txt=(char*)lines[i].text;
	char *ss;
	
	while ( (ss=strstr(txt, "---")) )
	{
	    int p=(int)ss - (int)lines[i].text;
	    
#define ISPG(c) ( ((c)=='-') || ((c)=='+') || ((c)=='\\') || ((((uint8_t)(c)) & 0xC0) == 0xC0) )
	    
	    // Ищем начало
	    while ( (p>0) && (ISPG(txt[p-1])) )
		p--;
	    
	    // Пребразуем
	    while ( (p < 64) && (ISPG(txt[p])) )
	    {
		uint8_t z=0;
		if ( (p>0) && (ISPG(txt[p-1])) ) z|=LEFT;
		if ( (p<63) && (ISPG(txt[p+1])) ) z|=RIGHT;
		if ( (i>0) && (ISPG(lines[i-1].text[p])) ) z|=TOP;
		if ( (i<n_lines-1) && (ISPG(lines[i+1].text[p])) ) z|=BOTTOM;
		
		txt[p]=pg[z];
		
		p++;
	    }
	}
	
	// Преобразуем '|'
	for (ss=txt; *ss; ss++)
	    if ((*ss)=='\\') (*ss)=pg[TOP | BOTTOM];
    }
#warning TODO: искать и преобразовывать таблицы
    
    
    // Совмещаем текст и аттрибуты
    for (int l=0; l<n_lines; l++)
    {
	uint8_t attr=0;
	uint8_t p=0;
	for (int x=0; (x<64) && (lines[l].text[x]); x++)
	{
	    if (lines[l].attr[x] != attr)
	    {
		// Замена аттрибута текста
		if (p >= 72)
		{
		    fprintf(stderr, "Line %d too long (after adding attrs)\n", l+1);
		    return -1;
		}
		attr=lines[l].attr[x];
		lines[l].enc[p++]=0x80 | attr;
	    }
	    
	    // Сам символ
	    if (p >= 72)
	    {
		fprintf(stderr, "Line %d too long (after adding attrs)\n", l+1);
		return -1;
	    }
	    lines[l].enc[p++]=lines[l].text[x];
	}
	
	// Если надо - отключаем аттрибуты в конце строки
	if (attr != 0)
	{
	    if (p >= 72)
	    {
		fprintf(stderr, "Line %d too long (after adding attrs)\n", l+1);
		return -1;
	    }
	    lines[l].enc[p++]=0x80;
	}
    }
    
    
    // Создаем таблицу строк
    uint16_t linetab[n_lines];
    linetab[0]=n_lines;	// первый элемент - кол-во строк, т.к. первая строка всегда с адреса 0
    int s=0;
    for (int i=0; i<n_lines; i++)
    {
	if (i!=0) linetab[i]=s;
	s+=strlen((const char*)lines[i].enc)+1;
    }
    
    
    // Сохраняем справку
    f=fopen(HELP_BIN, "wb");
    if (!f)
    {
	perror(HELP_BIN);
	return -1;
    }
    fwrite(linetab, 1, sizeof(linetab), f);
    for (int i=0; i<n_lines; i++)
    {
	fwrite(lines[i].enc, 1, strlen((const char*)lines[i].enc)+1, f);
    }
    fclose(f);
    
    return 0;
}
