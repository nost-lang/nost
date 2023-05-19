
#ifndef NOST_FN_H
#define NOST_FN_H

#include "vm.h"

// TODO: currently, an fn is a wrapper for a single bytecode value. if it remains this way, remove the fn type and just pass around bytecode for efficiency
typedef struct nost_fn {
    nost_obj obj; 
    nost_val bytecode;
} nost_fn;
nost_val nost_makeFn(nost_vm* vm, nost_val bytecode);

typedef struct nost_closure {
    nost_obj obj;
    nost_val fn;
    nost_val closureCtx;
} nost_closure;
nost_val nost_makeClosure(nost_vm* vm, nost_val fn, nost_val closureCtx);

typedef nost_val (*nost_fnPtr)(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args); 
typedef struct nost_natFn {
    nost_obj obj;
    nost_fnPtr fn;
} nost_natFn;

nost_val nost_makeNatFn(nost_vm* vm, nost_fnPtr fn);

#endif
