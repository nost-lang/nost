
#include "vm.h"
#include "gc.h"
#include "fiber.h"
#include "stdlib.h"
#include "pkg.h"

void nost_initVM(nost_vm* vm) {
    vm->heapAllocated = 0;
    vm->objsAllocated = 0;
#ifdef NOST_MEM_DEBUGGER
    vm->allocs = NULL;
#endif

    vm->gcPaused = 0;
    nost_gcPause(vm);
    vm->objs = NULL;
    nost_initDynarr(vm, &vm->grayObjs);
    nost_initDynarr(vm, &vm->blessed);
    nost_gcUnpause(vm);
    vm->rootCtx = nost_makeCtx(vm, NULL);
    nost_unbless(vm, (nost_obj*)vm->rootCtx);
    nost_initStdlib(vm);

    nost_initDynarr(vm, &vm->pkgs);
    nost_initDynarr(vm, &vm->pkgLoaders);
}

void nost_freeVM(nost_vm* vm) {
    nost_gcPause(vm);
    nost_freeDynarr(vm, &vm->grayObjs);
    nost_freeDynarr(vm, &vm->blessed);
    for(nost_obj* curr = vm->objs; curr != NULL;) {
        nost_obj* next = curr->next;
        nost_freeObj(vm, curr);
        curr = next;
    }
    nost_freeDynarr(vm, &vm->pkgs);
    nost_freeDynarr(vm, &vm->pkgLoaders);
}

void* nost_alloc(nost_vm* vm, size_t size) {
    return nost_realloc(vm, NULL, 0, size);
}

void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize) {
    nost_gc(vm); 
    vm->heapAllocated += newSize - oldSize;
    return realloc(ptr, newSize);
}

void nost_free(nost_vm* vm, void* ptr, size_t size) {
    nost_realloc(vm, ptr, size, 0);
}

#ifdef NOST_MEM_DEBUGGER

void* nost_dbgAlloc(nost_vm* vm, size_t size, const char* msg, const char* file, int line) {
    nost_allocDesc* desc = malloc(sizeof(nost_allocDesc));
    desc->msg = msg;
    desc->file = file;
    desc->line = line;
    desc->size = size;

    desc->prev = NULL;
    desc->next = vm->allocs;
    if(vm->allocs != NULL)
        vm->allocs->prev = desc;
    vm->allocs = desc;

    nost_allocDesc** alloc = malloc(size + sizeof(nost_allocDesc*));
    *alloc = desc;
    void* userAlloc = (char*)alloc + sizeof(nost_allocDesc*);
    vm->heapAllocated += size;

    return userAlloc;
}

void* nost_dbgRealloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize) {
    void* alloc = (void*)((char*)ptr - sizeof(nost_allocDesc*));
    nost_allocDesc* desc = *((nost_allocDesc**)alloc);
    if(newSize == 0) {
        if(desc->prev == NULL) {
            if(desc->next == NULL) {
                vm->allocs = NULL;
            } else {
                vm->allocs = desc->next;
                desc->next->prev = NULL;
            }
        } else {
            if(desc->next == NULL) {
                desc->prev->next = NULL;
            } else {
                desc->prev->next = desc->next;
                desc->next->prev = desc->prev;
            }
        }
        free(alloc);
        free(desc);
        vm->heapAllocated -= oldSize;
        return NULL;
    }
    alloc = realloc(alloc, newSize + sizeof(nost_allocDesc**));
    desc->size = newSize;
    vm->heapAllocated += newSize - oldSize;
    return (void*)((char*)alloc + sizeof(nost_allocDesc**));
}

void nost_dbgFree(nost_vm* vm, void* ptr, size_t size) {
    nost_dbgRealloc(vm, ptr, size, 0);
}

#endif

nost_pkg* nost_loadPkg(nost_vm* vm, nost_fiber* fiber, const char* name, nost_pkg* importFrom) {
    for(int i = 0; i < vm->pkgLoaders.cnt; i++) {
        nost_pkg* loadedPkg = vm->pkgLoaders.vals[i](vm, fiber, name, importFrom);
        if(loadedPkg != NULL)
            return loadedPkg;
    }
    return NULL;
}

void nost_addPkgLoader(nost_vm* vm, nost_pkgLoader loader) {
    nost_pushDynarr(vm, &vm->pkgLoaders, loader);
}
