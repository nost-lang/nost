
#include "fiber.h"
#include "gc.h"

nost_fiber* nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    nost_bless(vm, (nost_obj*)fiber);
    nost_initCtx(vm, &fiber->ctx);
    return fiber;
}

void nost_initCtx(nost_vm* vm, nost_ctx* ctx) {
    nost_initDynarr(vm, &ctx->dynvars);
}

static nost_val* getVarPtr(nost_fiber* fiber, nost_sym* name) {
    for(int i = 0; i < fiber->ctx.dynvars.cnt; i++) {
        if(nost_symEq(name, fiber->ctx.dynvars.vals[i].name))
            return &fiber->ctx.dynvars.vals[i].val;
    }
    return NULL;
}

nost_optVal nost_getVar(nost_fiber* fiber, nost_sym* name) {
    nost_val* ptr = getVarPtr(fiber, name);
    if(ptr != NULL)
        return nost_some(*ptr); 
    return nost_none(); 
}

bool nost_setVar(nost_fiber* fiber, nost_sym* name, nost_val val) {
    nost_val* ptr = getVarPtr(fiber, name);
    if(ptr == NULL)
        return false;
    *ptr = val;
    return true;
}

bool nost_addDynvar(nost_vm* vm, nost_fiber* fiber, nost_sym* name) {
    nost_val* valPtr = getVarPtr(fiber, name);
    if(valPtr == NULL) {
        nost_dynvar var;
        var.name = name;
        var.val = nost_nil();
        nost_pushDynarr(vm, &fiber->ctx.dynvars, var);
    }
    return valPtr == NULL; 
}

void nost_rtError(nost_fiber* fiber, nost_error err) {
    fiber->hadError = true;
    fiber->err = err;
}
