#include "common.h"
#include <string.h>
#include <stdlib.h>

char* add_suffix(const char* str, const char* suffix)
{
    const int str_len = strlen(str);
    const int new_len = str_len + strlen(suffix) + 2;
    char* result = calloc(new_len, sizeof(char));
    strcpy(result, str);
    result[str_len] = '-';
    strcpy(result + str_len + 1, suffix);
    return result;
}