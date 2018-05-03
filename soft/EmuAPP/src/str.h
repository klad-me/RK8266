#ifndef STR_H
#define STR_H


#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


int is_digit(char c);
int is_xdigit(char c);
int to_upper(int c);
int to_lower(int c);
char* ftoa(float num, uint8_t decimals);
char* itoa(int num);
int strnlen(const char *s, int maxlen);
int parse_hex(const char *s);


#ifdef __cplusplus
}
#endif


#endif
