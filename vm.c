
#include "vm.h"
#include "gc.h"

void nost_initVM(nost_vm* vm) {
    vm->gcPaused = true;
    vm->objs = NULL;
    nost_initDynarr(vm, &vm->grayObjs);
    nost_initDynarr(vm, &vm->blessed);
    vm->gcPaused = false;
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
