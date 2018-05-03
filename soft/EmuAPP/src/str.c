#include "str.h"

#include "ets.h"
#include "xprintf.h"


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
    xsprintf(pattern, "%%d.%%0%dd", decimals);
    xsprintf(buf, pattern, whole, decimal);
    return (char *)buf;
}


char* itoa(int num)
{
    xsprintf(buf, "%d", num);
    return buf;
}


int strnlen(const char *s, int maxlen)
{
    int l=0;
    for ( ; (*s) && (maxlen > 0); maxlen--, l++, s++);
    return l;
}


int parse_hex(const char *s)
{
    int x=0;
    
    while (*s)
    {
        if (! is_xdigit(*s)) break;
        x<<=4;
        if ( ((*s)>='A') && ((*s)<='F') ) x|=(*s)-'A'+10; else
        if ( ((*s)>='a') && ((*s)<='f') ) x|=(*s)-'a'+10; else
            x|=(*s)-'0';
	
        s++;
    }
    
    return x;
}
