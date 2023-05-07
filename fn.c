
#include "fn.h"

nost_val nost_makeNatFn(nost_vm* vm, nost_fnPtr fn) {
    nost_natFn* natFn = (nost_natFn*)nost_allocObj(vm, NOST_OBJ_NAT_FN, sizeof(nost_natFn));
    natFn->fn = fn; 
    return nost_objVal((nost_obj*)natFn);
}
