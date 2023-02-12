
#ifndef NOST_VM_H
#define NOST_VM_H

#include "common.h"
#include "error.h"
#include "obj.h"
#include "dynarr.h"
#include "sym.h"
#include "value.h"

typedef struct nost_vm {

} nost_vm;

void nost_initVM(nost_vm* vm);
void* nost_alloc(nost_vm* vm, size_t size);
void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize);
void nost_free(nost_vm* vm, void* ptr, size_t size);

typedef struct {
    nost_sym* name;
    nost_val val;
} nost_dynvar;

typedef struct {
    nost_dynarr(nost_dynvar) dynvars; 
    bool hadError;
    nost_error err;
} nost_ctx;

void nost_initCtx(nost_vm* vm, nost_ctx* ctx);
nost_optVal nost_getVar(nost_ctx* ctx, nost_sym* name);
bool nost_setVar(nost_ctx* ctx, nost_sym* name, nost_val val);
bool nost_addDynvar(nost_vm* vm, nost_ctx* ctx, nost_sym* name);
void nost_rtError(nost_ctx* ctx, nost_error err);

#endif
