
#ifndef NOST_MEM
#define NOST_MEM

#include "common.h"
#include "config.h"

struct nost_vm;

typedef struct nost_memTracker {
    const char* loc;
    size_t size;
    struct nost_memTracker* next;
} nost_memTracker;

#ifndef NOST_MEM_TRACK

void* nost_realloc(struct nost_vm* vm, void* ptr, size_t newSize);
void* nost_alloc(struct nost_vm* vm, size_t size);
void nost_free(struct nost_vm* vm, void* ptr);
#define NOST_REALLOC(vm, ptr, newSize) nost_realloc(vm, ptr, newSize)
#define NOST_ALLOC(vm, size) nost_alloc(vm, size)
#define NOST_FREE(vm, ptr) nost_free(vm, ptr)

void* nost_gcRealloc(struct nost_vm* vm, void* ptr, size_t oldSize, size_t newSize);
void* nost_gcAlloc(struct nost_vm* vm, size_t size);
void nost_gcFree(struct nost_vm* vm, void* ptr, size_t size);
#define NOST_GC_REALLOC(vm, ptr, oldSize, newSize) nost_gcRealloc(vm, ptr, oldSize, newSize)
#define NOST_GC_ALLOC(vm, size) nost_gcAlloc(vm, size)
#define NOST_GC_FREE(vm, ptr, size) nost_gcFree(vm, ptr, size)

void* nost_resAlloc(struct nost_vm* vm, size_t size, bool arena);
void nost_resFree(struct nost_vm* vm, void* ptr, size_t size);
#define NOST_RES_ALLOC(vm, size, arena) nost_resAlloc(vm, size, arena) 
#define NOST_RES_FREE(vm, ptr, size) nost_resFree(vm, ptr, size)

#else

void* nost_dbgRealloc(struct nost_vm* vm, void* ptr, size_t newSize, const char* loc);
void* nost_dbgAlloc(struct nost_vm* vm, size_t size, const char* loc);
void nost_dbgFree(struct nost_vm* vm, void* ptr, const char* loc);
#define NOST_REALLOC(vm, ptr, newSize) nost_dbgRealloc(vm, ptr, newSize, NOST_LOC())
#define NOST_ALLOC(vm, size) nost_dbgAlloc(vm, size, NOST_LOC())
#define NOST_FREE(vm, ptr) nost_dbgFree(vm, ptr, NOST_LOC())

void* nost_dbgGCRealloc(struct nost_vm* vm, void* ptr, size_t oldSize, size_t newSize, const char* loc);
void* nost_dbgGCAlloc(struct nost_vm* vm, size_t size, const char* loc);
void nost_dbgGCFree(struct nost_vm* vm, void* ptr, size_t size, const char* loc);
#define NOST_GC_REALLOC(vm, ptr, oldSize, newSize) nost_dbgGCRealloc(vm, ptr, oldSize, newSize, NOST_LOC())
#define NOST_GC_ALLOC(vm, size) nost_dbgGCAlloc(vm, size, NOST_LOC())
#define NOST_GC_FREE(vm, ptr, size) nost_dbgGCFree(vm, ptr, size, NOST_LOC())

void* nost_dbgResAlloc(struct nost_vm* vm, size_t size, bool arena, const char* loc);
void nost_dbgResFree(struct nost_vm* vm, void* ptr, size_t size, const char* loc);
#define NOST_RES_ALLOC(vm, size, arena) nost_dbgResAlloc(vm, size, arena, NOST_LOC()) 
#define NOST_RES_FREE(vm, ptr, size) nost_dbgResFree(vm, ptr, size, NOST_LOC())

#endif

typedef struct {
    uint8_t* base;
    uint8_t* curr;
    uint8_t* end;
    size_t size;
} nost_arena;

void nost_initArena(struct nost_vm* vm, nost_arena* arena, size_t size);
void nost_freeArena(struct nost_vm* vm, nost_arena* arena);
void* nost_arenaAlloc(nost_arena* arena, size_t size);
void nost_arenaClear(nost_arena* arena);

#endif
