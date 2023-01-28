#include "mystring.h"
#include <string.h>

void init_string(char* str, int size) {
    for(int k = 0; k < size; k++) {
        str[k] = '\0';
    }
}

void set_substr(char* src, char* dest, int from, int to) {
    if(from < 0 || to >= strlen(src)) {
        return;
    }

    int dest_i = 0;
    for(int k = from; k <= to; k++) {
        dest[dest_i] = src[k];
        dest_i++;
    }

}