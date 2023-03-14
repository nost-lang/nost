
#include "str.h"
#include "common.h"
#include "vm.h"

void nost_initStr(nost_vm* vm, nost_str* str) {
    str->str = NOST_ALLOC(vm, 8, "string");
    str->str[0] = '\0';
    str->len = 1;
    str->cap = 8;
}

int nost_write(nost_vm* vm, nost_str* str, char* fmt, ...) {

    va_list args;
    va_start(args, fmt); 
    va_list argsCopy;
    va_copy(argsCopy, args);

    size_t len = vsnprintf(NULL, 0, fmt, args);
    size_t oldLen = str->len;
    str->len += len;
    size_t initCap = str->cap;
    while(str->len > str->cap) {
        str->cap *= 2;
    }
    str->str = NOST_REALLOC(vm, str->str, initCap, str->cap);
    vsnprintf(str->str + oldLen - 1, len + 1, fmt, argsCopy);
    str->str[str->len] = '\0';

    va_end(args);
    va_end(argsCopy);

    return len;
}

void nost_freeStr(nost_vm* vm, nost_str* str) {
    NOST_FREE(vm, str->str, str->cap);
}
