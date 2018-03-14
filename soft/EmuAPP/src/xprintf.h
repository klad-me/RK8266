#ifndef XPRINTF_H
#define XPRINTF_H


#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif


int xvsprintf(char *buf, const char *fmt, va_list args);
int xsprintf(char *buf, const char *fmt, ...);


#ifdef __cplusplus
}
#endif


#endif
