
#ifndef NOST_VM_H
#define NOST_VM_H

#include "common.h"
#include "error.h"
#include "obj.h"
#include "dynarr.h"
#include "sym.h"
#include "value.h"

struct nost_ctx;
struct nost_fiber;
struct nost_pkg;

#ifdef NOST_MEM_DEBUGGER
typedef struct nost_allocDesc {
    const char* msg;
    const char* file;
    int line;
    size_t size;
    struct nost_allocDesc* prev;
    struct nost_allocDesc* next;
} nost_allocDesc;
#endif

typedef struct nost_pkg* (*nost_pkgLoader)(struct nost_vm* vm, struct nost_fiber* fiber, const char* name, struct nost_pkg* importFrom); 

typedef struct nost_vm {
    int gcPaused;
    nost_obj* objs;
    nost_dynarr(nost_obj*) grayObjs;
    nost_dynarr(nost_obj*) blessed;
    struct nost_ctx* rootCtx;
    size_t heapAllocated;
    int objsAllocated;

    nost_dynarr(struct nost_pkg*) pkgs;
    nost_dynarr(nost_pkgLoader) pkgLoaders;
#ifdef NOST_MEM_DEBUGGER
    nost_allocDesc* allocs; 
#endif

} nost_vm;

void nost_initVM(nost_vm* vm);
void nost_freeVM(nost_vm* vm);

void* nost_alloc(nost_vm* vm, size_t size);
void* nost_realloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize);
void nost_free(nost_vm* vm, void* ptr, size_t size);

#ifdef NOST_MEM_DEBUGGER

void* nost_dbgAlloc(nost_vm* vm, size_t size, const char* msg, const char* file, int line);
void* nost_dbgRealloc(nost_vm* vm, void* ptr, size_t oldSize, size_t newSize);
void nost_dbgFree(nost_vm* vm, void* ptr, size_t size);

#define NOST_ALLOC(vm, size, msg) nost_dbgAlloc(vm, (size), msg, __FILE__, __LINE__)
#define NOST_REALLOC(vm, ptr, oldSize, newSize) nost_dbgRealloc(vm, (ptr), (oldSize), (newSize)) 
#define NOST_FREE(vm, ptr, size) nost_dbgFree(vm, (ptr), (size));

#else

#define NOST_ALLOC(vm, size, msg) nost_alloc(vm, (size))
#define NOST_REALLOC(vm, ptr, oldSize, newSize) nost_realloc(vm, (ptr), (oldSize), (newSize))
#define NOST_FREE(vm, ptr, size) nost_free(vm, (ptr), (size));

#endif

struct nost_pkg* nost_loadPkg(nost_vm* vm, struct nost_fiber* fiber, const char* name, struct nost_pkg* importFrom);
void nost_addPkgLoader(nost_vm* vm, nost_pkgLoader loader); 

#endif
