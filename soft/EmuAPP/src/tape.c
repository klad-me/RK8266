#include "tape.h"

#include "ets.h"
#include "i8080_hal.h"
#include "ffs.h"
#include "ui.h"
#include "vg75.h"
#include "ps2.h"
#include "vg75.h"
#include "xprintf.h"


#define FLASH_TEMP_AT	0x60000
static uint8_t tape_buf[0x1000];



#define IN_SYNC_COUNT	16
#define IN_TIMEOUT	5000
static struct in
{
    uint32_t prev_cycles;
    uint8_t  bit, byte, bit_cnt;
    uint8_t  start, c;
    uint8_t  sync;
    uint16_t period;
    
    uint16_t size;
} in;


#define OUT_SYNC_COUNT	256
#define OUT_BIT_TIME	1600
static struct out
{
    uint16_t pos;
    uint32_t prev_cycles;
    uint32_t dataPtr;
    uint16_t dataPos;
    uint16_t dataSize;
    uint16_t sync;
    uint8_t  byte, bit;
    uint8_t  start, c, out;
} out;


void tape_init(void)
{
    in.prev_cycles=0;
    in.bit=0;
    in.byte=0;
    in.bit_cnt=0;
    in.start=0;
    in.c=0;
    in.sync=IN_SYNC_COUNT;
    in.period=0;
    in.size=0;
    
    out.prev_cycles=0;
    out.dataPtr=0;
    out.dataPos=0;
    out.dataSize=0;
    out.sync=0;
    out.byte=0;
    out.bit=0;
    out.start=0;
    out.out=0;
    out.pos=0;
}


static void tape_in_bit(void)
{
    // Добавляем бит
    in.byte=(in.byte << 1) | in.bit;
    in.bit_cnt++;
    
    if (in.bit_cnt==8)
    {
	// Выводим сообщение на экран
	if ((in.size & 31)==0)
	{
	    char str[32];
	    xsprintf(str, "Запись на магнитофон:  %d...", in.size);
	    vg75_overlay(str);
	}
	
	// Принят байт - кладем в буфер
	tape_buf[(in.size++) & 0xFFF]=in.byte;
	if ( (in.size & 0xFFF) == 0 )
	{
	    // Конец страницы - записываем ее на Flash
	    SPIUnlock();
	    SPIEraseSector( (FLASH_TEMP_AT + in.size - 0x1000) / 4096 );
	    SPIWrite(FLASH_TEMP_AT + in.size - 0x1000, tape_buf, 0x1000);
	}
	
	// Начинаем прием следующего байта
	in.byte=0;
	in.bit_cnt=0;
    }
}


void tape_in(void)
{
    // Получаем кол-во циклов с прошлого приема фронта
    uint32_t T=i8080_cycles - in.prev_cycles;
    in.prev_cycles=i8080_cycles;
    
    if (T > IN_TIMEOUT)
    {
	// Таймаут
	in.sync=IN_SYNC_COUNT;
	in.start=0;
	return;
    }
    
    // Синхронизируем тайминг
    if (in.sync > 0)
    {
	// Синхронизация
	if ((in.sync & 31)==0)
	{
	    char str[32];
	    xsprintf(str, "Загрузка с магнитофона...", in.size);
	    vg75_overlay(str);
	}
	
	if ( (T < in.period/2) || (T > in.period+in.period/2) )
	{
	    // Большая ошибка
	    in.period=T;
	    in.sync=IN_SYNC_COUNT;
	    in.start=0;
	    return;
	} else
	{
	    // Нормально
	    in.period=(in.period+T)/2;
	    in.sync--;
	}
    } else
    {
	uint8_t d;
	
	// Определяем - короткий или длинный период
	if ( (T < in.period/2) || (T > in.period*3) )
	{
	    // Большая ошибка
	    in.period=T;
	    in.sync=IN_SYNC_COUNT;
	    in.start=0;
	    return;
	} else
	if (T >= in.period+in.period/2)
	{
	    // Длинный
	    d=1;
	    in.period=(in.period+T/2)/2;
	} else
	{
	    // Короткий
	    d=0;
	    in.period=(in.period+T)/2;
	}
	
	// Обрабатываем данное
	if (! in.start)
	{
	    // Ждем стартовый бит
	    if (d)
	    {
		// Начало приема
		in.bit=1;
		in.start=1;
		in.c=1;
		in.byte=1;
		in.bit_cnt=1;
		in.size=0;
	    }
	} else
	{
	    // Идет прием
	    if (d)
	    {
		// Длинный - смена значения
		in.bit=in.bit ^ 1;
		in.c=1;
		tape_in_bit();
	    } else
	    {
		// Каждый второй короткий - повтор бита
		in.c^=1;
		if (in.c) tape_in_bit();
	    }
	}
    }
}


bool tape_out(void)
{
    uint32_t T=i8080_cycles - out.prev_cycles;
    
    // Ждем половины бита
    if (T < OUT_BIT_TIME/2)
    {
	// Пока мало времени прошло
	return out.out;
    }
    
    // Проверяем, если чтение из порта слишком медленное
    if (T < OUT_BIT_TIME)
	out.prev_cycles+=OUT_BIT_TIME/2; else
	out.prev_cycles=i8080_cycles;
    
    // Проверяем на конец передачи
    if (! out.start) return false;
    
    // Счетчик половины бита для манчестера
    out.c^=1;
    if (out.c) return out.out ^ 1;
    
    // Готовим новый байт, если надо
    if (out.bit==0)
    {
	if (out.sync > 0)
	{
	    // Синхронизация
	    if ((out.sync & 31)==0)
	    {
		vg75_overlay("Загрузка с магнитофона...");
	    }
	    
	    //ets_printf("TAPE: sync %d\n", out.sync);
	    out.byte=0x55;
	    out.bit=0x80;
	    out.sync--;
	} else
	{
	    // Данные
	    if ((out.dataPos & 31)==0)
	    {
		char str[48];
		xsprintf(str, "Загрузка с магнитофона:  %d/%d...", out.dataPos, out.dataSize);
		vg75_overlay(str);
	    }
	    
	    if (out.dataPos >= out.dataSize)
	    {
		// Конец данных
		vg75_overlay("Загрузка с магнитофона завершена.");
		ets_printf("TAPE: end\n");
		out.start=false;
		return false;
	    }
	    
	    // Проверим - остались ли данные в буфере
	    if (out.pos >= 0x1000)
	    {
		// Надо подгрузить данные из флэша в буфер
		uint16_t s=out.dataSize - out.dataPos;
		if (s > 0x1000)
		    s=0x1000; else
		    s=(s+3) & ~0x03;
		//ets_printf("TAPE: load from flash size=%d ptr=0x%05X buf=0x%08X\n", s, out.dataPtr, (uint32_t)out.buf);
		SPIRead(out.dataPtr, tape_buf, s);
		out.dataPtr+=s;
		out.pos=0;
	    }
	    
	    // Берем следующий байт из буфера
	    out.byte=tape_buf[out.pos++];
	    out.bit=0x80;
	    out.dataPos++;
	    //ets_printf("TAPE: data=0x%02X\n", out.byte);
	}
    }

    // Выталкиваем бит
    out.out=(out.byte & out.bit) ? 1 : 0;
    out.bit>>=1;
    
    return out.out;
}


bool tape_periodic(void)
{
    // Проверяем конец записи
    if (in.start)
    {
	// Идет запись, проверим на таймаут байт
	uint32_t T=i8080_cycles - in.prev_cycles;
	if (T > IN_TIMEOUT)
	{
	    // Таймаут - закончили прием
	    if ( (in.size & 0xFFF) != 0 )
	    {
		// Записываем остаток страницы во Flash
		SPIUnlock();
		SPIEraseSector( (FLASH_TEMP_AT + (in.size & ~0xFFF)) / 4096 );
		SPIWrite(FLASH_TEMP_AT + (in.size & ~0xFFF), tape_buf, 0x1000);
	    }
	    ets_printf("TAPE IN DONE size=%d !\n", in.size);
	    in.start=false;
	    return true;
	}
    }
    return false;
}


void tape_save(void)
{
    const char *name;
    
    // Получаем имя файла
again:
    name=ui_input_text("Введите имя файла для сохранения:", 0, 8);
    if ( (! name) || (! name[0]) ) return;
    
    // Ищем - вдруг такой файл уже есть
    if (ffs_find(name) >= 0)
    {
	// Уже есть такой файл
	if (ui_yes_no("Перезаписать файл ?")!=1) goto again;
	
	// Удаляем старый файл
	ffs_remove(ffs_find(name));
    }
    
    // Создаем файл
    screen.cursor_y=99;
    ui_draw_text(10, 12, "Запись в файл...");
    int16_t n=ffs_create(name, TYPE_TAPE, in.size);
    if (n < 0)
    {
	// Ошибка записи
	ui_draw_text(10, 12, "Ошибка записи в файл (диск полон?) !");
	ui_sleep(2000);
    }
    
    // Копируем данные
    uint16_t p=0;
    while (p < in.size)
    {
	uint16_t l=in.size-p;
	if (l > 0x1000) l=0x1000;
	
	SPIRead(FLASH_TEMP_AT + p, tape_buf, 0x1000);
	ffs_writeData(n, p, tape_buf, l);
	p+=l;
    }
}


void tape_load(uint16_t n)
{
    // Начинаем чтение
    out.dataPtr=ffs_flash_addr(n);
    out.dataPos=0;
    out.dataSize=fat[n].size;
    out.pos=0x1000;
    out.sync=OUT_SYNC_COUNT;
    out.start=1;
}
