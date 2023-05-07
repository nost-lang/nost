
#include "config.h"
#include "mem.h"
#include "vm.h"
#include "gc.h"

#ifndef NOST_MEM_TRACK

void* nost_realloc(nost_vm* vm, void* ptr, size_t newSize) {
    // TODO: make realloc customizable by embedding code
    (void)vm;
    return realloc(ptr, newSize);
}

void* nost_alloc(nost_vm* vm, size_t size) {
    return nost_realloc(vm, NULL, size); 
}

void nost_free(nost_vm* vm, void* ptr) {
    nost_realloc(vm, ptr, 0);
}


void* nost_gcRealloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize) {

    vm->liveHeap += newSize - oldSize;
#ifndef NOST_GC_STRESS
    if(vm->liveHeap > vm->gcThreshold) {
        nost_gc(vm);
        vm->gcThreshold = vm->liveHeap * 2;
    }
#else
    nost_gc(vm);
#endif

    return NOST_REALLOC(vm, ptr, newSize);
}

void* nost_gcAlloc(nost_vm* vm, size_t size) {
    return nost_gcRealloc(vm, NULL, 0, size);
}

void nost_gcFree(nost_vm* vm, void* ptr, size_t size) {
    nost_gcRealloc(vm, ptr, size, 0);
}

void* nost_resAlloc(struct nost_vm* vm, size_t size, bool arena) {
    if(arena) {
        void* alloc = nost_arenaAlloc(&vm->arena, size);
        if(alloc == NULL) {
            nost_gcArena(vm);
            void* newAlloc = nost_gcAlloc(vm, size); 
            return newAlloc;
        }
        return alloc;
    } else {
        void* alloc = nost_gcAlloc(vm, size); 
        return alloc;
    }
}

void nost_resFree(struct nost_vm* vm, void* ptr, size_t size) {
    nost_gcFree(vm, ptr, size);
}

#else

void* nost_dbgRealloc(nost_vm* vm, void* ptr, size_t newSize, const char* loc) {
    if(ptr == NULL) {
        nost_memTracker* track = malloc(sizeof(nost_memTracker));
        track->loc = loc;
        track->size = newSize;
        track->next = vm->trackers;
        vm->trackers = track;
        nost_memTracker** alloc = malloc(sizeof(nost_memTracker**) + newSize);
        *alloc = track;
        void* res = (uint8_t*)alloc + sizeof(nost_memTracker**);
        vm->allocatedMem += newSize;
        return res;
    }
    nost_memTracker** alloc = (nost_memTracker**)((uint8_t*)ptr - sizeof(nost_memTracker**));
    nost_memTracker* tracker = *alloc;
    vm->allocatedMem += newSize - tracker->size;
    tracker->size = newSize;
    alloc = realloc(alloc, sizeof(nost_memTracker**) + newSize);
    void* res = (uint8_t*)alloc + sizeof(nost_memTracker**);
    return res;
}

void* nost_dbgAlloc(nost_vm* vm, size_t size, const char* loc) {
    return nost_dbgRealloc(vm, NULL, size, loc); 
}

void nost_dbgFree(nost_vm* vm, void* ptr, const char* loc) {
    nost_dbgRealloc(vm, ptr, 0, loc);
}


void* nost_dbgGCRealloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize, const char* loc) {

    vm->liveHeap += newSize - oldSize;
#ifndef NOST_GC_STRESS
    if(vm->liveHeap > vm->gcThreshold) {
        nost_gc(vm);
        vm->gcThreshold = vm->liveHeap * 2;
    }
#else
    nost_gc(vm);
#endif

    return nost_dbgRealloc(vm, ptr, newSize, loc); 
}

void* nost_dbgGCAlloc(nost_vm* vm, size_t size, const char* loc) {
    return nost_dbgGCRealloc(vm, NULL, 0, size, loc);
}

void nost_dbgGCFree(nost_vm* vm, void* ptr, size_t size, const char* loc) {
    nost_dbgGCRealloc(vm, ptr, size, 0, loc);
}

void* nost_dbgResAlloc(nost_vm* vm, size_t size, bool arena, const char* loc) {
    if(arena) {
        const char** alloc = nost_arenaAlloc(&vm->arena, size + sizeof(const char*));
        if(alloc == NULL) {
            nost_gcArena(vm);
            void* newAlloc = nost_dbgGCAlloc(vm, size, loc); 
            return newAlloc;
        }
        *alloc = loc;
        uint8_t* res = (uint8_t*)alloc;
        res += sizeof(const char*);
        return res;
    } else {
        void* alloc = nost_dbgGCAlloc(vm, size, loc); 
        return alloc;
    }
}

void nost_dbgResFree(struct nost_vm* vm, void* ptr, size_t size, const char* loc) {
    (void)loc;
    NOST_GC_FREE(vm, ptr, size);
}

#endif

void nost_initArena(nost_vm* vm, nost_arena* arena, size_t size) {
    arena->base = arena->curr = NOST_GC_ALLOC(vm, size);
    arena->end = arena->base + size; 
    arena->size = size;
}

void nost_freeArena(struct nost_vm* vm, nost_arena* arena) {
    NOST_GC_FREE(vm, arena->base, 0); 
}

void* nost_arenaAlloc(nost_arena* arena, size_t size) {
    if(arena->curr + size >= arena->end)
        return NULL;
    void* res = arena->curr;
    arena->curr += size;
    return res;
}

void nost_arenaClear(nost_arena* arena) {

#ifdef NOST_STRICT_MODE
    // make sure code cannot rely on non-garbage values in areans after clear
    for(uint8_t* i = arena->base; i != arena->end; i++) {
        *i = rand();
    }
#endif

    arena->curr = arena->base;
}
