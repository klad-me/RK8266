#include "vg75.h"

#include <osapi.h>
#include "i8080_hal.h"
#include "tv_data.h"


static struct
{
    uint8_t cmd;
    uint8_t param[4], param_n;
} vg75;


static struct
{
    uint8_t param_n;
    uint8_t param[4];
} ik57;


void vg75_init(void)
{
    screen_w=78;
    screen_h=30;
    char_h=10;
    cursor_type=0;
    cursor_x=0;
    cursor_y=0;
    vram=i8080_hal_memory()+0x76D0;
}


void vg75_W(uint8_t A, uint8_t value)
{
    if (A)
    {
	// Команда
	vg75.cmd=value;
	vg75.param_n=0;
    } else
    {
	// Параметр
	vg75.param[vg75.param_n & 0x03]=value;
	vg75.param_n++;
	
	if ( (vg75.cmd==0x00) && (vg75.param_n==4) )
	{
	    // Сброс
	    screen_w=(vg75.param[0] & 0x7F)+1;	// число символов в строке -1
	    screen_h=(vg75.param[1] & 0x3F)+1;	// число строк на экране -1
	    char_h=(vg75.param[2] & 0x0F)+1;	// высота символа в пикселах -1
	    cursor_type=(vg75.param[3] >> 4) & 0x03;	// форма курсора: 0=мигающий блок, 1=мигающий штрих, 2=немигающий блок, 3=немигающий штрих
	    if (screen_w > 78) screen_w=78;
	    if (screen_h > 38) screen_h=38;
	    os_printf("VG75: W=%d H=%d CH=%d CUR=%d\n", screen_w, screen_h, char_h, cursor_type);
	} else
	if ( (vg75.cmd==0x80) && (vg75.param_n==2) )
	{
	    // Загрузить курсор
	    cursor_x=vg75.param[0];
	    cursor_y=vg75.param[1];
	    //os_printf("VG75: cursor=%d,%d\n", cursor_x, cursor_y);
	}
    }
}


uint8_t vg75_R(uint8_t A)
{
    return 0xff;
}


void ik57_W(uint8_t A, uint8_t value)
{
    if ( (A==0x08) && (value==0x80) )
    {
	// Команда инициализации
	ik57.param_n=0;
    } else
    if ( (A==0x04) || (A==0x05) )
    {
	ik57.param[ik57.param_n++]=value;
	
	if (ik57.param_n==4)
	{
	    uint16_t vram_at=(ik57.param[1] << 8) | ik57.param[0];
	    os_printf("IK57: VRAM @%04x\n", vram_at);
	    vram=i8080_hal_memory()+vram_at;
	    
	    ik57.param_n=0;
	}
    }
}


uint8_t ik57_R(uint8_t A)
{
    os_printf("IK57: read A=%02x\n", A);
    return 0x00;
}
