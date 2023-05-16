
#ifndef NOST_GC_H
#define NOST_GC_H

#include "vm.h"
#include "val.h"
#include "config.h"

// SAFETY-TODO: make a write barrier tracker

#ifndef NOST_BLESS_TRACK

nost_ref nost_pushBlessing(nost_vm* vm, nost_val val);
void nost_popBlessing(nost_vm* vm);

#define NOST_PUSH_BLESSING(vm, val) nost_pushBlessing(vm, val)
#define NOST_POP_BLESSING(vm, ref) nost_popBlessing(vm)

#else 

nost_ref nost_pushBlessing(nost_vm* vm, nost_val val, const char* loc);
void nost_popBlessing(nost_vm* vm, nost_ref ref);

#define NOST_PUSH_BLESSING(vm, val) nost_pushBlessing(vm, val, NOST_LOC())
#define NOST_POP_BLESSING(vm, ref) nost_popBlessing(vm, ref)

#endif

void nost_gcArena(nost_vm* vm);
void nost_gc(nost_vm* vm);

void nost_addObjToHeap(nost_vm* vm, nost_obj* obj);
void nost_writeBarrier(nost_vm* vm, nost_val refObj, nost_val newVal);

#endif
