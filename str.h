
#ifndef NOST_STR_H
#define NOST_STR_H

#include "vm.h"
#include "val.h"

typedef struct {
    char* str;
    size_t len;
    size_t cap;
} nost_str;

void nost_initStr(nost_str* str);
void nost_freeStr(nost_vm* vm, nost_str* str);
void nost_writeStr(nost_vm* vm, nost_str* str, const char* fmt, ...);
void nost_writeVal(nost_vm* vm, nost_str* str, nost_val val);

#endif
