
#ifndef NOST_FIBER_H
#define NOST_FIBER_H

#include "vm.h"

typedef struct {
    nost_sym* name;
    nost_val val;
} nost_dynvar;

typedef struct {
    nost_dynarr(nost_dynvar) dynvars; 
} nost_ctx;

void nost_initCtx(nost_vm* vm, nost_ctx* ctx);

typedef struct {
    nost_obj obj;
    nost_ctx ctx; 
    bool hadError;
    nost_error err;
} nost_fiber;

nost_fiber* nost_makeFiber(nost_vm* vm);

nost_optVal nost_getVar(nost_fiber* fiber, nost_sym* name);
bool nost_setVar(nost_fiber* fiber, nost_sym* name, nost_val val);
bool nost_addDynvar(nost_vm* vm, nost_fiber* fiber, nost_sym* name);
void nost_rtError(nost_fiber* ctx, nost_error err);

#endif
