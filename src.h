
#ifndef NOST_SRC_H
#define NOST_SRC_H

#include "val.h"
#include "vm.h"

typedef struct nost_src {
    nost_obj obj;
    char* src;
    size_t len;
} nost_src;

nost_val nost_makeSrc(nost_vm* vm);
void nost_initSrc(nost_vm* vm, nost_val src, const char* txt);

typedef struct nost_srcObj {
    nost_obj obj;
    nost_val src;
    nost_val val;
    int start;
    int end;
} nost_srcObj;

nost_val nost_makeSrcObj(nost_vm* vm, nost_ref src, nost_ref val, int start, int end);

nost_val nost_unwrap(nost_val val);

#endif
