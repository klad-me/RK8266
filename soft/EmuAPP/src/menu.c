#include "menu.h"

#include "ui.h"
#include "i8080.h"
#include "i8080_hal.h"
#include "reboot.h"
#include "ffs.h"
#include "files.h"
#include "tape.h"
#include "zkg.h"
#include "xprintf.h"
#include "fileman.h"


extern char __BUILD_NUMBER__;


bool menu_fileman(void)
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
	"4.Магнитофонные записи\n"
	);
    switch (ui_select(4))
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
	
	case 3:
	    // Магнитофонные записи
	    type=TYPE_TAPE;
	    break;
	
	default:
	    return false;
    }
    
    // Выбираем файл
again2:
    n=fileman(type, "Выберите файл для загрузки:");
    if (n < 0) goto again;
    
    // Загружаем файл
    if (type != TYPE_TAPE)
    {
	// Загрузка образа в память
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
	    goto again2;
	}
    } else
    {
	// Загрузка с магнитофона
	tape_load(n);
	return true;
    }
}




void menu(void)
{
    char str[32];
    
again:
    ui_clear();
    ui_header("РАДИО-86РК -->");
    ui_draw_list(
	"1.(  F11 )  Возврат в монитор (без очистки памяти)\n"
	"2.(  F12 )  Файловый менеджер\n"
	"3.(PrnScr) Полный сброс\n"
	"4.(Pause ) Переключиться в режим WiFi\n"
	);
    ui_draw_text(10, 16,
	"Привязка клавиатуры:\n"
	"Ф1-Ф4   - F1-F4          ВК  - Enter\n"
	"АР2     - Alt            ПС  - Доп. Enter\n"
	"РУС/ЛАТ - Caps Lock      ЗБ  - Backspace\n"
	"УС      - CTRL           \\   - Home\n"
	"СС      - Shift          СТР - End/Delete\n"
	"\n"
	"\n"
	"Управление эмуляцией:\n"
	"F5-F10      - Вызов ПЗУ E000+n*4\n"
	"Scroll Lock - Турбо режим\n"
	"WIN+Курсор  - Сдвиг изображения\n"
	"MENU        - Справка по Радио-86РК\n"
	);
    xsprintf(str, "RK8266 Сборка #%d", (int)&__BUILD_NUMBER__);
    ui_draw_text(64+6-ets_strlen(str), 33, str);
    switch (ui_select(4))
    {
	case 0:
	    // Возврат в монитор
	    i8080_jump(0xF800);
	    break;
	
	case 1:
	    // Файловый менеджер
	    if (! menu_fileman()) goto again;
	    break;
	
	case 2:
	    // Полный сброс
	    ets_memset(i8080_hal_memory(), 0x00, 0x8000);
	    i8080_init();
	    i8080_jump(0xF800);
	    break;
	
	case 3:
	    // Переключиться в режим WiFi
	    reboot(0x55AA55AA);
	    break;
    }
}
