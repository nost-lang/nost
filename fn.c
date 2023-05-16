
#include "fn.h"
#include "gc.h"

nost_val nost_makeFn(nost_vm* vm, nost_val bytecode) {
    nost_fn* fn = (nost_fn*)nost_allocObj(vm, NOST_OBJ_FN, sizeof(nost_fn));
    fn->bytecode = bytecode;
    return nost_objVal((nost_obj*)fn);
}

nost_val nost_makeClosure(nost_vm* vm, nost_val fn, nost_val closureCtx) {
    nost_ref fnRef = NOST_PUSH_BLESSING(vm, fn);
    nost_ref ctxRef = NOST_PUSH_BLESSING(vm, closureCtx);
    nost_closure* closure = (nost_closure*)nost_allocObj(vm, NOST_OBJ_CLOSURE, sizeof(nost_closure));
    closure->fn = nost_getRef(vm, fnRef);
    closure->closureCtx = nost_getRef(vm, ctxRef);
    NOST_POP_BLESSING(vm, ctxRef);
    NOST_POP_BLESSING(vm, fnRef);
    return nost_objVal((nost_obj*)closure);
}

nost_val nost_makeNatFn(nost_vm* vm, nost_fnPtr fn) {
    nost_natFn* natFn = (nost_natFn*)nost_allocObj(vm, NOST_OBJ_NAT_FN, sizeof(nost_natFn));
    natFn->fn = fn; 
    return nost_objVal((nost_obj*)natFn);
}
