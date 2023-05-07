
#include "vm.h"

void nost_initVM(nost_vm* vm) {

    vm->pauseGC = true;

#ifdef NOST_MEM_TRACK
    vm->allocatedMem = 0;
    vm->trackers = NULL;
#endif

    vm->objs = NULL;
    nost_initDynarr(&vm->grey);
#ifdef NOST_GREY_TRACK
    nost_initDynarr(&vm->greyLocs);
#endif
    nost_initDynarr(&vm->blessed);
    nost_initDynarr(&vm->heapToArenaRoots);
    vm->doingArenaGC = false;
    vm->liveHeap = 0;
    vm->gcThreshold = 2 * 1024 * 1024;
    nost_initArena(vm, &vm->arena, 100);

    vm->pauseGC = false;

}

void nost_freeVM(nost_vm* vm) {
    vm->pauseGC = true;
    for(nost_obj* obj = vm->objs; obj != NULL;) {
        nost_obj* next = obj->next;
        nost_freeObj(vm, obj);
        obj = next;
    }
    nost_freeArena(vm, &vm->arena);
    nost_freeDynarr(vm, &vm->blessed);
    nost_freeDynarr(vm, &vm->grey);
#ifdef NOST_GREY_TRACK
    nost_freeDynarr(vm, &vm->greyLocs);
#endif
    nost_freeDynarr(vm, &vm->heapToArenaRoots);
}
