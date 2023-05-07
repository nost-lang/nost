
#ifndef NOST_GC_H
#define NOST_GC_H

#include "vm.h"
#include "val.h"

// SAFETY-TODO: blessings *are* being leaked. make a blessing tracker to fix asap
// SAFETY-TODO: make a write barrier tracker

typedef int nost_ref;

nost_ref nost_pushBlessing(nost_vm* vm, nost_val val);
void nost_popBlessing(nost_vm* vm);

void nost_gcArena(nost_vm* vm);
void nost_gc(nost_vm* vm);

void nost_addObjToHeap(nost_vm* vm, nost_obj* obj);
void nost_writeBarrier(nost_vm* vm, nost_val refObj, nost_val newVal);

#endif
