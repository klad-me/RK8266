#include "menu.h"

#include "ui.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "reboot.h"
#include "ffs.h"
#include "files.h"
#include "tape.h"


static bool menu_load(void)
{
    uint8_t type;
    int16_t n;
    
again:
    ui_clear();
    ui_header("РАДИО-86РК -->");
    ui_draw_list(
	"1.Программы\n"
	"2.Игры\n"
	"3.Утилиты\n"
	);
    switch (ui_select(5))
    {
	case 0:
	    // Программы
	    type=TYPE_PROG;
	    break;
	
	case 1:
	    // Игры
	    type=TYPE_GAME;
	    break;
	
	case 2:
	    // Утилиты
	    type=TYPE_UTIL;
	    break;
	
	default:
	    return false;
    }
    
select_again:
    n=ui_select_file(type);
    if (n < 0) goto again;
    
    // Выбрали файл - загружаем
    int16_t addr=load_file(n);
    if (addr >= 0)
    {
	// Нормально загрузилось - запускаем
	i8080_jump(addr);
	
	// Возвращаемся к эмуляции
	return true;
    } else
    {
	// Ошибка загрузки файла
	ui_clear();
	ui_header("РАДИО-86РК -->");
	ui_draw_text(10, 10, "Ошибка загрузки файла !");
	ui_sleep(1000);
	goto select_again;
    }
}


void menu(void)
{
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
    switch (ui_select(5))
    {
	case 0:
	    // Возврат в монитор
	    i8080_jump(0xF800);
	    break;
	
	case 1:
	    // Полный сброс
	    ets_memset(i8080_hal_memory(), 0x00, 0x8000);
	    i8080_init();
	    i8080_jump(0xF800);
	    break;
	
	case 2:
	    // Загрузка образа
	    if (! menu_load()) goto again;
	    break;
	
	case 3:
	    // Загрузка с магнитофона
	    tape_load();
	    break;
	
	case 4:
	    // Переключиться в режим WiFi
	    reboot(0x55AA55AA);
	    break;
    }
}
