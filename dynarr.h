
#ifndef NOST_DYNARR_H
#define NOST_DYNARR_H

#define nost_dynarr(t) struct {t* vals; size_t cnt; size_t cap; size_t elemSize;}
#define nost_initDynarr(vm, arr) do {(arr)->elemSize = sizeof(*(arr)->vals); nost_initGenericDynarr(vm, arr);} while(0);
#define nost_pushDynarr(vm, arr, val) do {typeof(*(arr)->vals) XXXX = val; nost_pushGenericDynarr(vm, (arr), &XXXX);} while(0);
#define nost_clearDynarr(arr) (arr)->cnt = 0;
#define nost_freeDynarr(vm, arr)  

struct nost_vm;
void nost_initGenericDynarr(struct nost_vm* vm, void* arr);
void nost_pushGenericDynarr(struct nost_vm* vm, void* arr, void* val);

#endif
