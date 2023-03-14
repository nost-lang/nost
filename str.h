
#ifndef NOST_STR_H
#define NOST_STR_H

#include "common.h"

typedef struct {
    char* str;
    size_t len;
    size_t cap;
} nost_str;

struct nost_vm;
void nost_initStr(struct nost_vm* vm, nost_str* str);
int nost_write(struct nost_vm* vm, nost_str* str, char* fmt, ...);
void nost_freeStr(struct nost_vm* vm, nost_str* str);

#endif
