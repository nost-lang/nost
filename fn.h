
#ifndef NOST_FN_H
#define NOST_FN_H

#include "vm.h"

typedef nost_val (*nost_fnPtr)(nost_vm* vm, int nArgs, nost_val* args); 

typedef struct nost_natFn {
    nost_obj obj;
    nost_fnPtr fn;
} nost_natFn;

nost_val nost_makeNatFn(nost_vm* vm, nost_fnPtr fn);

#endif
