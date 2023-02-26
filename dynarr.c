
#include "dynarr.h"
#include "vm.h"

void nost_pushGenericDynarr(nost_vm* vm, void* arrPtr, void* val) {
    nost_dynarr(void)* arr = arrPtr;
    if(arr->cnt + 1 > arr->cap) {
        size_t oldSize = arr->cap * arr->elemSize;
        arr->cap *= 2;
        size_t newSize = arr->cap * arr->elemSize;
        arr->vals = NOST_REALLOC(vm, arr->vals, oldSize, newSize);
    }
    arr->cnt++;
    memcpy(arr->vals + (arr->cnt - 1) * arr->elemSize, val, arr->elemSize);
}
