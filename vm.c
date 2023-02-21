
#include "vm.h"
#include "gc.h"
#include "fiber.h"
#include "stdlib.h"

void nost_initVM(nost_vm* vm) {
    vm->gcPaused = 0;
    nost_gcPause(vm);
    vm->objs = NULL;
    nost_initDynarr(vm, &vm->grayObjs);
    nost_initDynarr(vm, &vm->blessed);
    nost_gcUnpause(vm);
    vm->rootCtx = nost_makeCtx(vm, NULL);
    nost_unbless(vm, (nost_obj*)vm->rootCtx);
    nost_initStdlib(vm);
}

void* nost_alloc(nost_vm* vm, size_t size) {
    return nost_realloc(vm, NULL, 0, size);
}

void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize) {
    nost_gc(vm); 
    return realloc(ptr, newSize);
}

void nost_free(nost_vm* vm, void* ptr, size_t size) {
    free(ptr);
}
