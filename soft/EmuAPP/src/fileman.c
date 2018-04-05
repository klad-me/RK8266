#include "fileman.h"

#include "ui.h"

#include "ets.h"
#include "ps2.h"
#include "ps2_codes.h"
#include "xprintf.h"
#include "xlat.h"
#include "ffs.h"
#include "vg75.h"


static void del(uint16_t n)
{
    char str[64];
    xsprintf(str, "Подтвердите удаление файла %s", ffs_name(n));
    if (ui_yes_no(str)==1)
    {
	ffs_remove(n);
    }
}


static void rename(uint16_t n)
{
    const char *name;
    
again:
    name=ui_input_text("Введите новое имя файла:", ffs_name(n), 8);
    if ( (! name) || (! name[0]) ) return;
    
    // Ищем - вдруг такой файл уже есть
    if (ffs_find(name) >= 0)
    {
        // Уже есть такой файл
        ui_draw_text(10, 12, "Файл с таким именем уже существует !");
        ui_sleep(2000);
        goto again;
    }
    
    // Переименовываем
    if (ffs_rename(n, name) < 0)
    {
        // Ошибка
        ui_draw_text(10, 12, "Ошибка переименования файла !");
        ui_sleep(2000);
    }
}


int16_t fileman(uint8_t type, const char *text)
{
#define MAX_FILES	80
    uint16_t filelist[MAX_FILES];
    int n_files;
    int i, j;
    int n=0, prev=0;
    
reread:
    // Собираем каталог файлов
    n_files=0;
    for (i=0; i<FAT_SIZE; i++)
    {
	if (fat[i].type == type)
	{
	    filelist[n_files++]=i;
	    if (n_files>=MAX_FILES) break;
	}
    }
    
    // Сортируем каталог по имени
    for (i=0; i<n_files-1; i++)
    {
	char tmp[10];
	ets_strcpy(tmp, ffs_name(filelist[i]));
	for (j=i+1; j<n_files; j++)
	{
	    if (ets_strcmp(tmp, ffs_name(filelist[j])) > 0)
	    {
		// Поменять местами
		uint16_t x=filelist[i];
		filelist[i]=filelist[j];
		filelist[j]=x;
	    }
	}
    }
    
    // Рисуем
    ui_clear();
    ui_header("РАДИО-86РК -->");
    if (n_files==0)
    {
	ui_draw_text(10, 10, "Нет файлов !");
	ui_sleep(1000);
	return -1;
    }
    ui_draw_text(10, 9, text);
    ui_draw_text(10, 32, "ENTER - выбор     ПРОБЕЛ - переименовать");
    ui_draw_text(10, 33, "ESC   - отмена    DELETE - удалить");
    
    // Для красивых рамок
    screen.underline_y=4;
#define PX	8
#define PY	10
    
    // Горизонтальные линии
    for (i=1; i<60; i++)
    {
	ui_scr[PY+ 0][PX+i]=0xE0;
	ui_scr[PY+21][PX+i]=0xE0;
    }
    
    // Вертикальные линии + аттрибуты
    for (i=0; i<20; i++)
    {
	for (j=0; j<4; j++)
	{
	    ui_scr[PY+1+i][PX+j*17+0]=0xE4;
	    ui_scr[PY+1+i][PX+j*17+1]=0x80;
	    ui_scr[PY+1+i][PX+j*17+16]=0x80;
	}
	
	ui_scr[PY+1+i][PX+4*17]=0xE4;
    }
    
    // Уголки
    ui_scr[PY+ 0][PX+0]=0xC0;
    ui_scr[PY+21][PX+0]=0xC8;
    ui_scr[PY+ 0][PX+60]=0xC4;
    ui_scr[PY+21][PX+60]=0xCC;
    
    // Рисуем список файлов
    for (i=0; i<n_files; i++)
    {
	int x=PX+1+(i/20)*17;
	int y=PY+1+(i%20);
	
	// Имя файла
	ui_draw_text(x+1, y, ffs_name(filelist[i]));
	
	// Размер
	char str[16];
	xsprintf(str, "%5d", fat[filelist[i]].size);
	ui_draw_text(x+10, y, str);
    }
    
    // Выбор файла
    if (n>=n_files) n=n_files-1;
    while (1)
    {
	// Стираем курсор с предыдущего файла
	ui_scr[PY+1+(prev%20)][PX+(prev/20)*17+1]=0x80;
	
	// Рисуем курсор на новом месте
	ui_scr[PY+1+(n%20)][PX+(n/20)*17+1]=0x90;
	
	// Запоминаем текущую позицию
	prev=n;
	
	// Обрабатываем нажатия кнопок
	while (1)
	{
	    uint16_t c=ps2_read();
	    if ( (c==PS2_UP) && (n>0) )
	    {
		// Вверх
		n--;
		break;
	    } else
	    if ( (c==PS2_DOWN) && (n<n_files-1) )
	    {
		// Вниз
		n++;
		break;
	    } else
	    if ( (c==PS2_LEFT) && (n>0) )
	    {
		// Влево
		n-=20;
		if (n<0) n=0;
		break;
	    } else
	    if ( (c==PS2_RIGHT) && (n<n_files-1) )
	    {
		// Вправо
		n+=20;
		if (n>=n_files) n=n_files-1;
		break;
	    } else
	    if ( (c==PS2_DELETE) || (c==PS2_D) || (c==PS2_BACKSPACE) )
	    {
		// Удалить
		del(filelist[n]);
		goto reread;
	    } else
	    if ( (c==PS2_SPACE) || (c==PS2_R) )
	    {
		// Переименовать
		rename(filelist[n]);
		goto reread;
	    } else
	    if ( (c==PS2_ENTER) || (c==PS2_KP_ENTER) )
	    {
		// Выбрать
		n=filelist[n];
		goto done;
	    } else
	    if (c==PS2_ESC)
	    {
		// Отмена
		n=-1;
		goto done;
	    }
	}
    }
    
    
done:
    // Возвращаем позицию подчеркивания
    screen.underline_y=7;
    
    return n;
}
