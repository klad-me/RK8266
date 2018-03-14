#include "str.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <mem.h>


int is_digit(char c)
{
    return ( (c>='0') && (c<='9') );
}


int is_xdigit(char c)
{
    return
	( (c>='0') && (c<='9') ) ||
	( (c>='A') && (c<='F') ) ||
	( (c>='a') && (c<='f') );
}


int to_upper(int c)
{
    if ( (c>='a') && (c<='z') )
	return (c-'a')+'A'; else
	return c;
}


int to_lower(int c)
{
    if ( (c>='A') && (c<='Z') )
	return (c-'A')+'a'; else
	return c;
}


/*char* strcat(char *dst, const char *src)
{
    os_memcpy(dst+os_strlen(dst), src, os_strlen(src)+1);
    return dst;
}*/


char* strdup(const char *str)
{
    char *data=(char*)os_malloc(os_strlen(str)+1);
    os_strcpy(data, str);
    return data;
}


// Нужно для SDK v1.5.3
char* stpcpy(char *dest, const char *src)
{
    while (*src) (*dest++)=(*src++);
    (*dest)=0;
    return dest;
}


char* os_strdup(const char *str)
{
    if (!str) str="";
    int l=os_strlen(str)+1;
    char *s=(char*)os_malloc(l);
    if (s) os_memcpy(s, str, l);
    return s;
}


void os_strdel(const char *str)
{
    os_free((void*)str);
}


static int power(int base, int exp)
{
    int result = 1;
    while(exp) { result *= base; exp--; }
    return result;
}


static char buf[16];

char* ftoa(float num, uint8_t decimals)
{
    // float to string; no float support in esp8266 sdk printf
    // warning: limited to 15 chars & non-reentrant
    // e.g., dont use more than once per os_printf call
    int whole = num;
    int decimal = (num - whole) * power(10, decimals);
    if (decimal < 0)
    {
	// get rid of sign on decimal portion
	decimal -= 2 * decimal;
    }
    char pattern[10]; // setup printf pattern for decimal portion
    os_sprintf(pattern, "%%d.%%0%dd", decimals);
    os_sprintf(buf, pattern, whole, decimal);
    return (char *)buf;
}


char* itoa(int num)
{
    os_sprintf(buf, "%d", num);
    return buf;
}


const char* skip_ws(const char *text)
{
    for ( ; ((*text)==' ') || ((*text)=='\t') || ((*text)=='\r') || ((*text)=='\n'); text++);
    return text;
}


int os_strnlen(const char *s, int maxlen)
{
    int l=0;
    for ( ; (*s) && (maxlen > 0); maxlen--, l++, s++);
    return l;
}


int parse_int(const char *s)
{
    int v=0, sign=1;
    
    if ((*s)=='+') s++; else
    if ((*s)=='-')
    {
        s++;
        sign=-1;
    }
    
    while ( ((*s)>='0') && ((*s)<='9') )
        v=v*10+(*(s++))-'0';
    
    return v*sign;
}
