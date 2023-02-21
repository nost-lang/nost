
#include "fiber.h"
#include "gc.h"

nost_fiber* nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    nost_bless(vm, (nost_obj*)fiber);
    nost_initDynarr(vm, &fiber->frames);
    nost_pushFrame(vm, fiber, vm->rootCtx);
    return fiber;
}

nost_frame* nost_currFrame(nost_fiber* fiber) {
    return &fiber->frames.vals[fiber->frames.cnt - 1];
}

void nost_pushFrame(nost_vm* vm, nost_fiber* fiber, nost_ctx* parentCtx) {
    nost_gcPause(vm);
    nost_frame frame;
    nost_pushDynarr(vm, &fiber->frames, frame); 
    nost_frame* newFrame = nost_currFrame(fiber);
    newFrame->currCtx = nost_makeCtx(vm, parentCtx);
    nost_gcUnpause(vm);
}

void nost_popFrame(nost_vm* vm, nost_fiber* fiber) {
    nost_popDynarr(&fiber->frames);
}

nost_ctx* nost_currCtx(nost_fiber* fiber) {
    return nost_currFrame(fiber)->currCtx;
}

void nost_pushCtx(nost_vm* vm, nost_fiber* fiber) {
    nost_ctx* newCtx = nost_makeCtx(vm, nost_currCtx(fiber));
    nost_currFrame(fiber)->currCtx = newCtx;
}

void nost_popCtx(nost_fiber* fiber) {
    nost_currFrame(fiber)->currCtx = nost_currCtx(fiber)->parent;
}

nost_ctx* nost_makeCtx(nost_vm* vm, nost_ctx* parent) {
    nost_gcPause(vm);
    nost_ctx* ctx = (nost_ctx*)nost_allocObj(vm, NOST_OBJ_CTX, sizeof(nost_ctx));
    nost_initDynarr(vm, &ctx->dynvars);
    ctx->parent = parent;
    nost_gcUnpause(vm);
    return ctx;
}

static nost_val* getVarPtrInCtx(nost_ctx* ctx, nost_sym* name) {
    for(int i = 0; i < ctx->dynvars.cnt; i++) {
        if(nost_symEq(name, ctx->dynvars.vals[i].name))
            return &ctx->dynvars.vals[i].val;
    }
    return NULL;
}

static nost_val* getVarPtr(nost_ctx* ctx, nost_sym* name) {
    nost_ctx* curr = ctx; 
    while(curr != NULL) {
        nost_val* varPtr = getVarPtrInCtx(curr, name);
        if(varPtr != NULL)
            return varPtr;
        curr = curr->parent;
    }
    return NULL;
}

nost_optVal nost_getVarInCtx(nost_ctx* ctx, nost_sym* name) {
    nost_val* ptr = getVarPtr(ctx, name);
    if(ptr != NULL)
        return nost_some(*ptr); 
    return nost_none(); 
}

bool nost_setVarInCtx(nost_ctx* ctx, nost_sym* name, nost_val val) {
    nost_val* ptr = getVarPtr(ctx, name);
    if(ptr == NULL)
        return false;
    *ptr = val;
    return true;
}

bool nost_addDynvarInCtx(nost_vm* vm, nost_ctx* ctx, nost_sym* name) {
    nost_val* valPtr = getVarPtrInCtx(ctx, name);
    if(valPtr == NULL) {
        nost_dynvar var;
        var.name = name;
        var.val = nost_nil();
        nost_pushDynarr(vm, &ctx->dynvars, var);
    }
    return valPtr == NULL; 
}

nost_optVal nost_getVar(nost_fiber* fiber, nost_sym* name) {
    return nost_getVarInCtx(nost_currCtx(fiber), name);
}

bool nost_setVar(nost_fiber* fiber, nost_sym* name, nost_val val) {
    return nost_setVarInCtx(nost_currCtx(fiber), name, val);
}

bool nost_addDynvar(nost_vm* vm, nost_fiber* fiber, nost_sym* name) {
    return nost_addDynvarInCtx(vm, nost_currCtx(fiber), name);
}

void nost_rtError(nost_fiber* fiber, nost_error err) {
    fiber->hadError = true;
    fiber->err = err;
}
