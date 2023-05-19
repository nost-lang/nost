
#ifndef NOST_FIBER_H
#define NOST_FIBER_H

#include "val.h"
#include "vm.h"
#include "dynarr.h"
#include "error.h"

typedef struct {
    nost_val name;
    nost_val val;
} nost_dynvar;

typedef struct nost_ctx {
    nost_obj obj;
    nost_gcDynarr(nost_dynvar) vars;
    nost_val parent;
} nost_ctx;

nost_val nost_makeCtx(nost_vm* vm, nost_val parent);

void nost_addDynvar(nost_vm* vm, nost_ref ctx, nost_val name, nost_val val); 
nost_val nost_getVar(bool* valid, nost_val ctx, nost_val name);

typedef struct {
    nost_val ctx;
    nost_val bytecode;
    int ip;
} nost_frame;

typedef struct nost_fiber {
    nost_obj obj;
    nost_gcDynarr(nost_val) stack;
    nost_gcDynarr(nost_frame) frames;

    nost_error err;
    bool hadError;
} nost_fiber;

nost_val nost_makeFiber(nost_vm* vm);
nost_val nost_execBytecode(nost_vm* vm, nost_ref fiber, nost_val bytecode, nost_val ctx);

void nost_natFnError(nost_vm* vm, nost_ref fiber, const char* fmt, ...);

#endif
