
#ifndef NOST_SYM_H
#define NOST_SYM_H

#include "obj.h"
#include "common.h"
#include "value.h"

typedef struct nost_sym {
    nost_obj obj;
    size_t len;
    char* sym;
} nost_sym;

nost_val nost_makeSym(struct nost_vm* vm, const char* sym, size_t len);
bool nost_symEq(nost_sym* a, nost_sym* b);
bool nost_symIs(nost_val val, const char* str);

#endif
