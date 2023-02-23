
#ifndef NOST_DYNARR_H
#define NOST_DYNARR_H

#define nost_dynarr(t) struct {t* vals; size_t cnt; size_t cap; size_t elemSize;}
#define nost_initDynarr(vm, arr) do { \
        (arr)->elemSize = sizeof(*(arr)->vals); \
        (arr)->cnt = 0; \
        (arr)->cap = 8; \
        (arr)->vals = NOST_ALLOC(vm, (arr)->cap * (arr)->elemSize, "dynarr"); \
    } while(0);
#define nost_pushDynarr(vm, arr, val) do {typeof(*(arr)->vals) XXXX = val; nost_pushGenericDynarr(vm, (arr), &XXXX);} while(0);
#define nost_clearDynarr(arr) (arr)->cnt = 0;
#define nost_popDynarr(arr) (arr)->cnt--;
#define nost_freeDynarr(vm, arr) NOST_FREE(vm, (arr)->vals, (arr)->cap * (arr)->elemSize); 

struct nost_vm;
void nost_pushGenericDynarr(struct nost_vm* vm, void* arr, void* val);

#endif
