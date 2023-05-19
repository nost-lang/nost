
#include "list.h"
#include "gc.h"
#include "src.h"

bool nost_isConslike(nost_val val) {
    val = nost_unwrap(val);
    return nost_isCons(val);
}

bool nost_refIsConslike(nost_vm* vm, nost_ref ref) {
    return nost_isConslike(nost_getRef(vm, ref));
}

nost_val nost_car(nost_val val) {
    return nost_asCons(nost_unwrap(val))->car;
}

nost_val nost_cdr(nost_val val) {
    return nost_asCons(nost_unwrap(val))->cdr;
}

nost_val nost_refCar(nost_vm* vm, nost_ref ref) {
    return nost_refAsCons(vm, ref)->car; 
}

nost_val nost_refCdr(nost_vm* vm, nost_ref ref) {
    return nost_refAsCons(vm, ref)->cdr;
}

void nost_setCdr(nost_vm* vm, nost_val cons, nost_val val) {
    nost_writeBarrier(vm, cons, val);
    nost_asCons(cons)->cdr = val;
}

int nost_listLen(nost_val list) {
    int len = 0;
    while(!nost_isNil(list)) {
        len++;
        list = nost_cdr(list);
    }
    return len;
}

nost_val nost_nth(nost_vm* vm, nost_val list, int n) {
    nost_ref ref = NOST_PUSH_BLESSING(vm, list);
    for(int i = 0; i < n; i++) {
        nost_setRef(vm, ref, nost_refCdr(vm, ref));
    }
    nost_val res = nost_refCar(vm, ref);
    NOST_POP_BLESSING(vm, ref);
    return res;
}
