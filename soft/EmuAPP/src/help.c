#include "help.h"

#include "ets.h"
#include "vg75.h"
#include "ui.h"
#include "ps2.h"
#include "ps2_codes.h"


#define HELP_AT	0x50000


static int n_lines=-1, line=0;


static void display(void)
{
    int addr=0;
    
    // Читаем адрес первой видимой строки
    if (line != 0)
    {
	uint16_t tmp[2];
	SPIRead(HELP_AT+(line & 0xFFFE)*2, (uint32_t*)tmp, 4);	// чтение должно быть выравнено по границе 4 байт
	addr=tmp[line & 1];
    }
    
    // Добавляем смещение таблицы строк и адрес начала во флэше
    addr+=n_lines*2 + HELP_AT;
    //ets_printf("HELP: display start 0x%05X\n", addr);
    
    // Рисуем строки
    int i;
    for (i=0; i<30; i++)
    {
	// Проверяем на конец текста
	if (line+i >= n_lines) break;
	
	//ets_printf("HELP: line %d at 0x%05X\n", line+i, addr);
	
	// Читаем текст в буфер
	uint8_t buf[80];	// макс. длина - 72 символа + на выравнивание по границе 4 байт
	SPIRead(addr & ~0x03, (uint32_t*)buf, sizeof(buf));
	uint8_t o=addr & 0x03;
	uint8_t x=8;
	while (buf[o])
	{
	    ui_scr[4+i][x++]=buf[o++];
	    addr++;
	}
	
	// Очищаем остаток строки
	while (x < 80)
	    ui_scr[4+i][x++]=0;
	
	addr++;	// конец строки
    }
}


void help_display(void)
{
    ui_start();
    
    // Читаем кол-во строк (если не читали еще)
    if (n_lines < 0)
    {
	uint16_t tmp[2];
	
	SPIRead(HELP_AT, (uint32_t*)tmp, 4);
	n_lines=tmp[0];
	//ets_printf("HELP: n_lines=%d\n", n_lines);
    }
    
draw:
    // Рисуем на экране
    display();
    
    // Обрабатываем кнопки
    while (1)
    {
	uint16_t c=ps2_read();
	
	switch (c)
	{
	    case PS2_ESC:
	    case PS2_MENU:
		// Выход
		goto done;
	    
	    case PS2_UP:
		// Вверх
		if (line > 0)
		{
		    line--;
		    goto draw;
		}
		break;
	    
	    case PS2_DOWN:
		// Вниз
		if (line+30 < n_lines)
		{
		    line++;
		    goto draw;
		}
		break;
	    
	    case PS2_PGUP:
		// Страница вверх
		if (line > 0)
		{
		    line-=29;
		    if (line < 0) line=0;
		    goto draw;
		}
		break;
	    
	    case PS2_PGDN:
		// Страница вниз
		if (line+30 < n_lines)
		{
		    line+=29;
		    if (line+30 >= n_lines)
			line=n_lines-30;
		    goto draw;
		}
		break;
	    
	    case PS2_HOME:
		// Начало
		if (line > 0)
		{
		    line=0;
		    goto draw;
		}
		break;
	    
	    case PS2_END:
		// Конец
		if (line+30 < n_lines)
		{
		    line=n_lines-30;
		    goto draw;
		}
		break;
	}
    }
    
done:
    ui_stop();
}
