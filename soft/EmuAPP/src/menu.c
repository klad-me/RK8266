#include "menu.h"

#include "ui.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "reboot.h"
#include "ffs.h"
#include "files.h"


#define MAX_FILES	80
static uint8_t load_type;
static uint8_t filelist[MAX_FILES];
static uint8_t n_files;


static PT_THREAD(menu_load_select(struct pt *pt))
{
    static struct pt sub;
    static uint32_t _sleep;
    uint16_t i;
    
    PT_BEGIN(pt);
	PT_YIELD(pt);
	
	// Собираем каталог файлов
	n_files=0;
	for (i=0; i<FAT_SIZE; i++)
	{
	    if (fat[i].type == load_type)
	    {
		filelist[n_files++]=i;
		if (n_files>=MAX_FILES) break;
	    }
	}
	
again:
	// Рисуем
	ui_clear();
	ui_header("РАДИО-86РК -->");
	if (n_files==0)
	{
	    ui_draw_text(10, 10, "Нет файлов !");
	    PT_SLEEP(1000000);
	    PT_EXIT(pt);
	}
	
	ui_draw_text(10, 8, "Выберите файл для загрузки:");
	for (i=0; i<n_files; i++)
	{
	    ui_draw_text(10+(i/20)*16, 10+(i%20), ffs_name(filelist[i]));
	}
	ui_select_n=0; ui_select_count=n_files;
	PT_SUB(ui_select);
	
	if (ui_select_n >= 0)
	{
	    // Выбрали файл - загружаем
	    int16_t addr=load_file(filelist[ui_select_n]);
	    if (addr >= 0)
	    {
		// Нормально загрузилось - запускаем
		i8080_jump(addr);
		
		// Возвращаемся к эмуляции
		ui_select_n=-1;
		PT_EXIT(pt);
	    } else
	    {
		// Ошибка загрузки файла
		ui_clear();
		ui_header("РАДИО-86РК -->");
		ui_draw_text(10, 10, "Ошибка загрузки файла !");
		PT_SLEEP(1000000);
		goto again;
	    }
	}
	
	// Отмена
	ui_select_n=0;	// чтобы не возвращаться в эмулятор
    PT_END(pt);
}


static PT_THREAD(menu_load(struct pt *pt))
{
    static struct pt sub;
    
    PT_BEGIN(pt);
	PT_YIELD(pt);
again:
	ui_clear();
	ui_header("РАДИО-86РК -->");
	ui_draw_list(
	    "1.Программы\n"
	    "2.Игры\n"
	    "3.Утилиты\n"
	    );
	ui_select_n=0; ui_select_count=3;
	PT_SUB(ui_select);
	if (ui_select_n==0)
	{
	    // Программы
	    load_type=TYPE_PROG;
	    PT_SUB(menu_load_select);
	} else
	if (ui_select_n==1)
	{
	    // Игры
	    load_type=TYPE_GAME;
	    PT_SUB(menu_load_select);
	} else
	if (ui_select_n==2)
	{
	    // Утилиты
	    load_type=TYPE_UTIL;
	    PT_SUB(menu_load_select);
	}
	
	if (ui_select_n < 0)
	{
	    // Выход
	    PT_EXIT(pt);
	}
	
	goto again;
    PT_END(pt);
}


PT_THREAD(menu(struct pt *pt))
{
    static struct pt sub;
    
    PT_BEGIN(pt);
	PT_YIELD(pt);
again:
	ui_clear();
	ui_header("РАДИО-86РК -->");
	ui_draw_list(
	    "1.Возврат в монитор (без очистки памяти)\n"
	    "2.Полный сброс\n"
	    "3.Загрузка образа\n"
	    "4.Загрузка с магнитофона\n"
	    "5.Переключиться в режим WiFi\n"
	    );
	ui_draw_text(10, 20,
	    "Привязка клавиатуры:\n"
	    "Ф1-Ф4   - F1-F4\n"
	    "ВК      - Enter\n"
	    "ПС      - Enter на доп.клавиатуре\n"
	    "ЗБ      - Backspace\n"
	    "УС      - CTRL\n"
	    "СС      - Shift\n"
	    "РУС/ЛАТ - Caps Lock\n"
	    "\\       - Home\n"
	    "СТР     - End/Delete\n"
	    "АР2     - Alt\n"
	    );
	ui_select_n=0; ui_select_count=5;
	PT_SUB(ui_select);
	// Тут switch делать нельзя из-за pt
	if (ui_select_n==0)
	{
	    // Возврат в монитор
	    i8080_jump(0xF800);
	    PT_EXIT(pt);
	} else
	if (ui_select_n==1)
	{
	    // Полный сброс
	    ets_memset(i8080_hal_memory(), 0x00, 0x8000);
	    i8080_init();
	    i8080_jump(0xF800);
	    PT_EXIT(pt);
	} else
	if (ui_select_n==2)
	{
	    // Загрузка образа
	    PT_SUB(menu_load);
	} else
	if (ui_select_n==3)
	{
	    // Загрузка с магнитофона
	} else
	if (ui_select_n==4)
	{
	    // Переключиться в режим WiFi
	    reboot(0x55AA55AA);
	}
	
	if (ui_select_n < 0)
	{
	    // Выход
	    PT_EXIT(pt);
	}
	
	goto again;
    PT_END(pt);
}
