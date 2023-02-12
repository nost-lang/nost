
#include "dynarr.h"
#include "vm.h"

void nost_initGenericDynarr(nost_vm* vm, void* arrPtr) {
    nost_dynarr(void)* arr = arrPtr;
    arr->cnt = 0;
    arr->cap = 8;
    arr->vals = nost_alloc(vm, arr->cap * arr->elemSize); 
}

void nost_pushGenericDynarr(nost_vm* vm, void* arrPtr, void* val) {
    nost_dynarr(void)* arr = arrPtr;
    arr->cnt++;
    if(arr->cnt > arr->cap) {
        size_t oldSize = arr->cap * arr->elemSize;
        arr->cap *= 2;
        size_t newSize = arr->cap * arr->elemSize;
        arr->vals = nost_realloc(vm, arr->vals, oldSize, newSize);
    }
    memcpy(arr->vals + (arr->cnt - 1) * arr->elemSize, val, arr->elemSize);
}
