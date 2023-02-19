
#include "fiber.h"
#include "gc.h"

nost_fiber* nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    nost_bless(vm, (nost_obj*)fiber);
    fiber->currCtx = nost_makeCtx(vm, NULL);
    return fiber;
}

nost_ctx* nost_currCtx(nost_fiber* fiber) {
    return fiber->currCtx;
}

void nost_pushCtx(nost_vm* vm, nost_fiber* fiber) {
    nost_ctx* newCtx = nost_makeCtx(vm, nost_currCtx(fiber));
    fiber->currCtx = newCtx;
}

void nost_popCtx(nost_fiber* fiber) {
    fiber->currCtx = nost_currCtx(fiber)->parent;
}

nost_ctx* nost_makeCtx(nost_vm* vm, nost_ctx* parent) {
    nost_gcPause(vm);
    nost_ctx* ctx = (nost_ctx*)nost_allocObj(vm, NOST_OBJ_CTX, sizeof(nost_ctx));
    nost_initDynarr(vm, &ctx->dynvars);
    ctx->parent = parent;
    nost_gcUnpause(vm);
    return ctx;
}

static nost_val* getVarPtr(nost_fiber* fiber, nost_sym* name) {
    nost_ctx* curr = nost_currCtx(fiber); 
    while(curr != NULL) {
        for(int i = 0; i < curr->dynvars.cnt; i++) {
            if(nost_symEq(name, curr->dynvars.vals[i].name))
                return &curr->dynvars.vals[i].val;
        }
        curr = curr->parent;
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
        nost_ctx* ctx = nost_currCtx(fiber);
        nost_pushDynarr(vm, &ctx->dynvars, var);
    }
    return valPtr == NULL; 
}

void nost_rtError(nost_fiber* fiber, nost_error err) {
    fiber->hadError = true;
    fiber->err = err;
}
