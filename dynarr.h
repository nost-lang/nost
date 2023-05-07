
#ifndef NOST_DYNARR_H
#define NOST_DYNARR_H

#include "mem.h"

#define NOST_DYNARR_START_CAP 8

#define nost_dynarr(t) struct {int cnt; int cap; t* vals;}
#define nost_initDynarr(arr) do {(arr)->cnt = 0; (arr)->cap = 0; (arr)->vals = NULL;} while(0);
#define nost_pushDynarr(vm, arr, val) \
    do { \
        if((arr)->cap == 0) { \
            (arr)->cap = NOST_DYNARR_START_CAP; \
            (arr)->vals = NOST_ALLOC(vm, (arr)->cap * sizeof(*(arr)->vals)); \
        } \
        (arr)->cnt++; \
        if((arr)->cnt >= (arr)->cap) { \
            (arr)->cap *= 2; \
            (arr)->vals = NOST_REALLOC(vm, (arr)->vals, (arr)->cap * sizeof(*(arr)->vals)); \
        } \
        (arr)->vals[(arr)->cnt - 1] = val; \
    } while(0);
#define nost_freeDynarr(vm, arr) NOST_FREE(vm, (arr)->vals);

// NOTE: cap is called gcCap to detect if normal dynarr passed instead of gcDynarr
#define nost_gcDynarr(t) struct {int cnt; int gcCap; t* vals; bool onArena;}
#define nost_initGCDynarr(arr) do {(arr)->cnt = 0; (arr)->gcCap = 0; (arr)->vals = NULL; (arr)->onArena = true;} while(0);
#define nost_moveGCDynarr(src, dest) do {(dest)->cnt = (src)->cnt; (dest)->gcCap = (src)->gcCap; (dest)->vals = (src)->vals; (dest)->onArena = (src)->onArena;} while(0);
// NOTE: dest used because src might be moved in the middle of the push
#define nost_gcPushDynarr(vm, srcPtr, val, dest) \
    do { \
        nost_gcDynarr(typeof(*((srcPtr)->vals))) src; \
        nost_moveGCDynarr(srcPtr, &src); \
        int oldCap = src.gcCap; \
        if(src.gcCap == 0) \
            src.gcCap = NOST_DYNARR_START_CAP;  \
        if(src.gcCap == src.cnt) \
            src.gcCap *= 2; \
        if(src.gcCap != oldCap) { \
            size_t oldSize = oldCap * sizeof(*src.vals); \
            size_t newSize = src.gcCap * sizeof(*src.vals); \
            if(src.onArena) { \
                void* alloc = nost_arenaAlloc(&vm->arena, newSize);  \
                if(alloc == NULL) { \
                    void* heapAlloc = NOST_GC_ALLOC(vm, newSize); \
                    memcpy(heapAlloc, src.vals, oldSize); \
                    src.vals = heapAlloc; \
                    src.onArena = false; \
                    nost_gcArena(vm); \
                } else { \
                    memcpy(alloc, src.vals, oldSize); \
                    src.vals = alloc; \
                } \
            } else { \
                src.vals = NOST_GC_REALLOC(vm, src.vals, oldSize, newSize); \
            } \
        }  \
        src.cnt++; \
        src.vals[src.cnt - 1] = (val); \
        nost_moveGCDynarr(&src, dest); \
    } while(0);
#define nost_freeGCDynarr(vm, arr) if(!(arr)->onArena) { NOST_GC_FREE(vm, (arr)->vals, (arr)->gcCap * sizeof(*(arr)->vals)); }
#define nost_heapifyGCDynarr(vm, arr) \
    do { \
        size_t valsSize = sizeof(*(arr)->vals) * (arr)->gcCap; \
        void* heapVals = NOST_GC_ALLOC(vm, valsSize); \
        memcpy(heapVals, (arr)->vals, valsSize); \
        (arr)->vals = heapVals; \
        (arr)->onArena = false; \
    } while(0);

#define nost_popDynarr(vm, arr) (arr)->cnt--;

#endif
