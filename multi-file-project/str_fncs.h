#ifndef _STR_FNCS_H_
#define _STR_FNCS_H_

#include <stdbool.h>

bool isalldigits(char *str);
char *next_token(char **str_ptr, const char *delim);

#endif
