
#ifndef NOST_VM
#define NOST_VM

#include "mem.h"
#include "val.h"
#include "dynarr.h"

typedef struct nost_vm {
    nost_arena arena; 

    nost_obj* objs;
    nost_dynarr(nost_val*) grey;
#ifdef NOST_GREY_TRACK
    nost_dynarr(const char*) greyLocs;
#endif
    nost_dynarr(nost_val) blessed;
    nost_dynarr(nost_val) heapToArenaRoots;
    size_t liveHeap;
    size_t gcThreshold;
    bool doingArenaGC;

    // NOTE: only use in exceptional circumstances. only pauses mark sweep collection.
    bool pauseGC; 

#ifdef NOST_MEM_TRACK
    size_t allocatedMem;
    nost_memTracker* trackers;
#endif

} nost_vm;

void nost_initVM(nost_vm* vm);
void nost_freeVM(nost_vm* vm);

#endif
