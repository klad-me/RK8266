#ifndef LIBESP_STR_H
#define LIBESP_STR_H


#include <os_type.h>


#ifdef __cplusplus
extern "C" {
#endif


int is_digit(char c);
int is_xdigit(char c);
int to_upper(int c);
int to_lower(int c);
char* ftoa(float num, uint8_t decimals);
char* itoa(int num);
char* os_strdup(const char *str);
void os_strdel(const char *str);
const char* skip_ws(const char *text);
int os_strnlen(const char *s, int maxlen);
int parse_int(const char *s);


#ifdef __cplusplus
}
#endif


#endif
