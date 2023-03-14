
#include "util.h"

int nost_findLineBegin(const char* str, int idx) {
    if(str[idx] == '\n')
        idx--;
    while(str[idx] != '\n' && idx != 0)
        idx--;
    if(str[idx] == '\n')
        idx++;
    return idx;
}

int nost_findLineEnd(const char* str, int idx) {
    while(str[idx] != '\n' && str[idx] != '\0')
        idx++;
    return idx;
}
