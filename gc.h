
#ifndef NOST_GC_H
#define NOST_GC_H

#include "obj.h"
#include "value.h"
#include "vm.h"


void nost_bless(nost_vm* vm, nost_obj* obj);
void nost_blessVal(nost_vm* vm, nost_val val);
void nost_unbless(nost_vm* vm, nost_obj* obj);
void nost_unblessVal(nost_vm* vm, nost_val val);

void nost_gcPause(nost_vm* vm);
void nost_gcUnpause(nost_vm* vm);

void nost_gc(nost_vm* vm);

#endif
