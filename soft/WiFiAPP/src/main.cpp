#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include "Thread.h"

#include "gpio_lib.h"
#include "tv.h"
#include "tv_data.h"
#include "timer0.h"
#include "ps2.h"
#include "ps2_codes.h"
#include "ui.h"
#include "ffs.h"
#include "HTTPd.h"
#include "Web.h"
#include "reboot.h"
#include "board.h"


THREAD(status_thread)
{
    char str[51], ss[32];
    
    PT_BEGIN(pt);
	while (1)
	{
	    // Делаем пустую строку
	    os_memset(str, ' ', sizeof(str));
	    str[50]=0;
	    
	    // Заголовок
	    os_strcpy(str+0, "РАДИО-86РК -->");
	    
	    // Состояние подключения к WiFi
	    switch (wifi_station_get_connect_status())
	    {
		case STATION_IDLE:
		    os_strcpy(ss, "Нет подключения к WiFi");
		    break;
		
		case STATION_CONNECTING:
		    os_strcpy(ss, "Подключение к WiFi");
		    break;
		
		case STATION_WRONG_PASSWORD:
		    os_strcpy(ss, "Неверный пароль WiFi");
		    break;
		
		case STATION_NO_AP_FOUND:
		    os_strcpy(ss, "Нет сети WiFi");
		    break;
		
		case STATION_CONNECT_FAIL:
		    os_strcpy(ss, "Ошибка подключения к WiFi");
		    break;
		
		case STATION_GOT_IP:
		    {
			struct station_config cfg;
			struct ip_info ipi;
			wifi_station_get_config(&cfg);
			wifi_get_ip_info(STATION_IF, &ipi);
			os_sprintf(ss, "WiFi: %s: %d.%d.%d.%d",
			    cfg.ssid,
			    ip4_addr1(&ipi.ip), ip4_addr2(&ipi.ip), ip4_addr3(&ipi.ip), ip4_addr4(&ipi.ip) );
		    }
		    break;
		
		default:
		    os_strcpy(ss, "WiFi отключен");
		    break;
	    }
	    
	    // Состояние WiFi справа
	    os_strcpy(str+(50-os_strlen(ss)), ss);
	    
	    // Копируем на экран
	    os_memcpy(&vram[0][0], str, 50);
	    
	    PT_SLEEP(200);
	}
    PT_END(pt);
}



static bool scan_done_flag=true;
struct AP
{
    char ssid[32];
    int  rssi;
    uint8_t auth;
};
#define MAX_AP	16
static struct AP aplist[MAX_AP];
static uint8_t n_ap;


static void scan_done(void *arg, STATUS status)
{
    struct bss_info *bss=(struct bss_info*)arg;
    
    // Добавляем все точки доступа в список
    n_ap=0;
    while ( (bss) && (n_ap < MAX_AP) )
    {
	os_strcpy(aplist[n_ap].ssid, (char*)bss->ssid);
	aplist[n_ap].rssi=bss->rssi;
	aplist[n_ap].auth=bss->authmode;
	n_ap++;
	
	bss=bss->next.stqe_next;
    }
    
    // Сортируем точки доступа по rssi
    for (uint8_t i=0; i<n_ap-1; i++)
    {
	for (uint8_t j=i+1; j<n_ap; j++)
	{
	    if (aplist[i].rssi < aplist[j].rssi)
	    {
		// Меняем местами
		struct AP tmp;
		tmp=aplist[i];
		aplist[i]=aplist[j];
		aplist[j]=tmp;
	    }
	}
    }
    
    scan_done_flag=true;
}


static PT_THREAD(wifi_scan(struct pt *pt))
{
    static struct pt sub;
    static char password[32];
    static uint8_t edit_pos;
    char c;
    
    PT_BEGIN(pt);
	// Рисуем ожидание
	ui_clear();
	os_strcpy(&vram[3][0], "Поиск WiFi-сетей...");
again:
	// Проверим, что прошлый поиск закончился
	if (scan_done_flag)
	{
	    // Запускаем поиск
	    scan_done_flag=false;
	    if (! wifi_station_scan(NULL, scan_done)) PT_EXIT(pt);
	}
	
	// Ждем окончание сканирования или отмену
	while (! scan_done_flag)
	{
	    if (ps2_sym()==KEY_ESC)
	    {
		// Отмена
		PT_EXIT(pt);
	    }
	    
	    PT_SLEEP(200);
	}
	
	// Если ничего не нашлось - повторяем еще раз
	if (n_ap==0) goto again;
	
select_ssid:
	// Отображаем список доступных точек доступа
	cursor_y=99;
	ui_clear();
	os_strcpy(&vram[2][0], "Выберите сеть для подключения:");
	for (uint8_t i=0; i<n_ap; i++)
	{
	    static const char *authname[]=
		{
		    "Открытая",
		    "WEP",
		    "WPA",
		    "WPA2",
		    "WPA/WPA2"
		};
	    
	    // SSID
	    os_strcpy(&vram[4+i][4], aplist[i].ssid);
	    
	    // RSSI
	    os_sprintf(&vram[4+i][34], "%3d dBm", aplist[i].rssi);
	    
	    // AUTH
	    os_strcpy(&vram[4+i][42], authname[aplist[i].auth]);
	}
	
	// Делаем выбор
	ui_select_n=0;
	ui_select_count=n_ap;
	PT_SUB(ui_select);
	if (ui_select_n < 0) PT_EXIT(pt);
	
	// Проверим тип шифрования
	if (aplist[ui_select_n].auth==AUTH_OPEN)
	{
	    // Открытая сеть - пароль не нужен
	    wifi_station_disconnect();
	    PT_SLEEP(100);
	    
	    struct station_config cfg;
	    os_strcpy((char*)cfg.ssid, aplist[ui_select_n].ssid);
	    cfg.password[0]=0;
	    cfg.bssid_set=0;
	    
	    wifi_station_set_config(&cfg);
	    wifi_station_set_config_current(&cfg);
	    wifi_station_connect();
	    
	    PT_EXIT(pt);
	}
	
	
	// Запрашиваем пароль
	ui_clear();
	os_sprintf(&vram[2][0], "Введите пароль для сети '%s':", aplist[ui_select_n].ssid);
	os_strcpy(&vram[3][0], "Пароль: ");
#define EDIT_X	8
#define EDIT_Y	3
	cursor_x=EDIT_X;
	cursor_y=EDIT_Y;
	edit_pos=0;
	while (1)
	{
	    c=ps2_sym();
	    
	    if (c==KEY_ESC) goto select_ssid; else
	    if ( (c==KEY_ENTER) && (edit_pos > 0) )
	    {
		// Настраиваем WiFi
		wifi_station_disconnect();
		PT_SLEEP(100);
		
		password[edit_pos]=0;
		
		struct station_config cfg;
		os_strcpy((char*)cfg.ssid, aplist[ui_select_n].ssid);
		os_strcpy((char*)cfg.password, password);
		cfg.bssid_set=0;
		
		wifi_station_set_config(&cfg);
		wifi_station_set_config_current(&cfg);
		wifi_station_connect();
		
		PT_EXIT(pt);
	    } else
	    if ( (c==KEY_BACKSPACE) && (edit_pos > 0) )
	    {
		// Удалить символ
		edit_pos--;
		vram[EDIT_Y][EDIT_X+edit_pos]=' ';
		cursor_x=EDIT_X+edit_pos;
	    } else
	    if ( (c>=32) && (edit_pos < 31) )
	    {
		// Вставить символ
		vram[EDIT_Y][EDIT_X+edit_pos]=c;
		password[edit_pos]=c;
		edit_pos++;
		cursor_x=EDIT_X+edit_pos;
	    }
	    
	    PT_SLEEP(100);
	}
#undef EDIT_Y
#undef EDIT_X
    PT_END(pt);
}


static void font_tab(void)
{
    static const char *hex="0123456789ABCDEF";
    
    for (int y=0; y<16; y++)
    {
	vram[2][4+y*2]=hex[y];
	vram[4+y][1]=hex[y];
	for (int x=0; x<16; x++)
	{
	    vram[4+y][4+x*2]=y*16+x;
	}
    }
}


THREAD(main_thread)
{
    static struct pt sub;
    char c;
    
    PT_BEGIN(pt);
	// Пишем информацию
redraw:
	cursor_y=99;
	ui_clear();
	os_strcpy(&vram[3][0], "Нажмите ESC для возврата в режим эмуляции.");
	os_strcpy(&vram[4][0], "Нажмите Пробел для выбора WiFi-сети.");
	while (1)
	{
	    if (wifi_station_get_connect_status() == STATION_GOT_IP)
	    {
		// Есть соединение
		char ss[32];
		struct ip_info ipi;
		wifi_get_ip_info(STATION_IF, &ipi);
		os_sprintf(ss, "http://%d.%d.%d.%d/",
		    ip4_addr1(&ipi.ip), ip4_addr2(&ipi.ip), ip4_addr3(&ipi.ip), ip4_addr4(&ipi.ip) );
		
		ui_draw_text(0, 7,
		    "Для обмена файлами, откройте браузер и в строке\n"
		    "адреса укажите:");
		os_strcpy(&vram[9][5], ss);
	    } else
	    {
		// Нет соединения - очищаем строки
		os_memset(&vram[7][0], 0x00, 50*3);
	    }
	    
	    c=ps2_sym();
	    if (c==KEY_ESC)
	    {
		reboot(0);
	    } else
	    if (c==' ')
	    {
		PT_SUB(wifi_scan);
		goto redraw;
	    }
	    
	    //os_printf("Free heap size:%d\n", system_get_free_heap_size());
	    PT_SLEEP(200);
	}
    PT_END(pt);
}


THREAD(heap_thread)
{
    PT_BEGIN(pt);
        while (1)
        {
            PT_SLEEP(1000);
            os_printf("Free heap size:%d\n", system_get_free_heap_size());
        }
    PT_END(pt);
}


static void RAMFUNC my_putc1(char c)
{
}


extern "C" void user_init(void);
void ICACHE_FLASH_ATTR user_init()
{
    // Переводим проц на 160МГц
    system_update_cpu_freq(SYS_CPU_160MHZ);
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    os_delay_us(100000);
    
#ifndef DO_DEBUG
    // Заменяем os_printf на себя
    os_install_putc1((void*)my_putc1);
#endif
    
    os_printf("\n\n\n");
    os_printf("Free heap size: %d\n", system_get_free_heap_size());
    os_printf("SDK version:%s\n", system_get_sdk_version());
    
    // Входы-выходы
    os_printf("[GPIO]\n");
    gpio_init();
    
    // Нитки
    os_printf("[Threads]\n");
    threads_init();
    
    // Файлы
    os_printf("[FFS]\n");
    ffs_init();
    
    // HTTP-сервер
    os_printf("[HTTPd]\n");
    (new HTTPd(80))->setHandler(new Web());
    
    // Клавиатура
    os_printf("[PS2]\n");
    ps2_init();
    
    // TV-out
    os_printf("[TV]\n");
    tv_init();
    
    
    /*uint32_t prev_T=getCycleCount();
    while (1)
    {
	uint32_t T=getCycleCount();
	
	if ((uint32_t)(T-prev_T) >= 160000000)
	{
	    os_printf("Alive dbg1=%d dbg2=%d d=%d\n", dbg1, dbg2, dbg1-dbg2);
	    prev_T=T;
	}
	
	tv_data_periodic();
	//os_printf("ints=%d\n", n_ints);
	//os_delay_us(200000);
	
	system_soft_wdt_feed();
    }*/
    
    // Включаем режим станции
    wifi_set_opmode_current(STATION_MODE);
    wifi_set_opmode(STATION_MODE);
    
    // Задача строки статуса
    status_thread();
    
    // Меню
    main_thread();
    
    
    heap_thread();
}
