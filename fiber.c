
#include "fiber.h"
#include "gc.h"
#include "pkg.h"

nost_fiber* nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    nost_bless(vm, (nost_obj*)fiber);
    nost_initDynarr(vm, &fiber->frames);
    nost_pushFrame(vm, fiber, vm->rootCtx, NULL, nost_nil());
    return fiber;
}

nost_frame* nost_currFrame(nost_fiber* fiber) {
    return &fiber->frames.vals[fiber->frames.cnt - 1];
}

void nost_pushFrameWithCtx(nost_vm* vm, nost_fiber* fiber, nost_ctx* ctx, nost_pkg* pkg, nost_val callsite) {
    nost_gcPause(vm);
    nost_frame frame;
    nost_pushDynarr(vm, &fiber->frames, frame); 
    nost_frame* newFrame = nost_currFrame(fiber);
    newFrame->currCtx = ctx;
    newFrame->pkg = pkg;
    newFrame->callsite = callsite;
    nost_gcUnpause(vm);
}

void nost_pushFrame(nost_vm* vm, nost_fiber* fiber, nost_ctx* parentCtx, nost_pkg* pkg, nost_val callsite) {
    nost_gcPause(vm);
    nost_pushFrameWithCtx(vm, fiber, nost_makeCtx(vm, parentCtx), pkg, callsite);
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
    return nost_addDynvarInCtxWithDecl(vm, ctx, name, nost_nil());
}

bool nost_addDynvarInCtxWithDecl(nost_vm* vm, nost_ctx* ctx, nost_sym* name, nost_val decl) {
    nost_gcPause(vm);
    nost_val* valPtr = getVarPtrInCtx(ctx, name);
    if(valPtr == NULL) {
        nost_dynvar var;
        var.name = name;
        var.val = nost_nil();
        var.decl = decl; 
        nost_pushDynarr(vm, &ctx->dynvars, var);
    }
    nost_gcUnpause(vm);
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

bool nost_addDynvarWithDecl(nost_vm* vm, nost_fiber* fiber, nost_sym* name, nost_val decl) {
    return nost_addDynvarInCtxWithDecl(vm, nost_currCtx(fiber), name, decl);
}

nost_val nost_getVarDecl(nost_vm* vm, nost_fiber* fiber, nost_sym* name) {
    nost_ctx* curr = nost_currCtx(fiber); 
    while(curr != NULL) {
        for(int i = 0; i < curr->dynvars.cnt; i++)
            if(nost_symEq(curr->dynvars.vals[i].name, name))
                return curr->dynvars.vals->decl;
        curr = curr->parent;
    }
    return nost_nil();
}

void nost_rtError(nost_vm* vm, nost_fiber* fiber, nost_error err) {
    nost_gcPause(vm);
    fiber->hadError = true;
    if(fiber->frames.cnt > 1) {
        nost_addMessage(vm, &err, "\nStack trace:");
        for(int i = fiber->frames.cnt - 1; i >= 0; i--) {
            nost_frame* frame = &fiber->frames.vals[i];
            if(nost_isNil(frame->callsite))
                break;
            nost_addValRef(vm, &err, frame->callsite);
        }
    }
    fiber->err = err;
    nost_gcUnpause(vm);
}
