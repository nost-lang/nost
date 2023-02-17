
#ifndef NOST_VM_H
#define NOST_VM_H

#include "common.h"
#include "error.h"
#include "obj.h"
#include "dynarr.h"
#include "sym.h"
#include "value.h"

typedef struct nost_vm {
    bool gcPaused;
    nost_obj* objs;
    nost_dynarr(nost_obj*) grayObjs;
    nost_dynarr(nost_obj*) blessed;
} nost_vm;

void nost_initVM(nost_vm* vm);
void* nost_alloc(nost_vm* vm, size_t size);
void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize);
void nost_free(nost_vm* vm, void* ptr, size_t size);


#endif
