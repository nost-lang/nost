
#ifndef NOST_LIST_H
#define NOST_LIST_H

#include "vm.h"

// TODO: move all the list stuff from val.h here

bool nost_isConslike(nost_val val);
bool nost_refIsConslike(nost_vm* vm, nost_ref ref);

nost_val nost_car(nost_val val);
nost_val nost_cdr(nost_val val);
nost_val nost_refCar(nost_vm* vm, nost_ref ref);
nost_val nost_refCdr(nost_vm* vm, nost_ref ref);
void nost_setCdr(nost_vm* vm, nost_val cons, nost_val val);

int nost_listLen(nost_val list);
nost_val nost_nth(nost_vm* vm, nost_val list, int n);

#endif
