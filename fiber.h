
#ifndef NOST_FIBER_H
#define NOST_FIBER_H

#include "vm.h"

typedef struct {
    nost_sym* name;
    nost_val val;
} nost_dynvar;

typedef struct nost_ctx {
    nost_obj obj;
    nost_dynarr(nost_dynvar) dynvars; 
    struct nost_ctx* parent;
} nost_ctx;

nost_ctx* nost_makeCtx(nost_vm* vm, nost_ctx* parent);
nost_optVal nost_getVarInCtx(nost_ctx* ctx, nost_sym* name);
bool nost_setVarInCtx(nost_ctx* ctx, nost_sym* name, nost_val val);
bool nost_addDynvarInCtx(nost_vm* vm, nost_ctx* ctx, nost_sym* name);

typedef struct {
    nost_ctx* currCtx;
} nost_frame;

typedef struct {
    nost_obj obj;
    nost_dynarr(nost_frame) frames;
    bool hadError;
    nost_error err;
} nost_fiber;

nost_fiber* nost_makeFiber(nost_vm* vm);

nost_frame* nost_currFrame(nost_fiber* fiber);
void nost_pushFrame(nost_vm* vm, nost_fiber* fiber, nost_ctx* parentCtx);
void nost_popFrame(nost_vm* vm, nost_fiber* fiber);

nost_ctx* nost_currCtx(nost_fiber* fiber);
void nost_pushCtx(nost_vm* vm, nost_fiber* fiber);
void nost_popCtx(nost_fiber* fiber);

nost_optVal nost_getVar(nost_fiber* fiber, nost_sym* name);
bool nost_setVar(nost_fiber* fiber, nost_sym* name, nost_val val);
bool nost_addDynvar(nost_vm* vm, nost_fiber* fiber, nost_sym* name);

void nost_rtError(nost_fiber* fiber, nost_error err);

#endif
