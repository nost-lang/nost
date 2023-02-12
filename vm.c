
#include "vm.h"

void nost_initVM(nost_vm* vm) {

}

void* nost_alloc(nost_vm* vm, size_t size) {
    return malloc(size);
}

void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize) {
    return realloc(ptr, newSize);
}

void nost_free(nost_vm* vm, void* ptr, size_t size) {
    free(ptr);
}

void nost_initCtx(nost_vm* vm, nost_ctx* ctx) {
    ctx->hadError = false;
    nost_initDynarr(vm, &ctx->dynvars);
}

static nost_val* getVarPtr(nost_ctx* ctx, nost_sym* name) {
    for(int i = 0; i < ctx->dynvars.cnt; i++) {
        if(nost_symEq(name, ctx->dynvars.vals[i].name))
            return &ctx->dynvars.vals[i].val;
    }
    return NULL;
}

nost_optVal nost_getVar(nost_ctx* ctx, nost_sym* name) {
    nost_val* ptr = getVarPtr(ctx, name);
    if(ptr != NULL)
        return nost_some(*ptr); 
    return nost_none(); 
}

bool nost_setVar(nost_ctx* ctx, nost_sym* name, nost_val val) {
    nost_val* ptr = getVarPtr(ctx, name);
    if(ptr == NULL)
        return false;
    *ptr = val;
    return true;
}

bool nost_addDynvar(nost_vm* vm, nost_ctx* ctx, nost_sym* name) {
    nost_val* valPtr = getVarPtr(ctx, name);
    if(valPtr == NULL) {
        nost_dynvar var;
        var.name = name;
        var.val = nost_nil();
        nost_pushDynarr(vm, &ctx->dynvars, var);
    }
    return valPtr == NULL; 
}

void nost_rtError(nost_ctx* ctx, nost_error err) {
    ctx->hadError = true;
    ctx->err = err;
}
