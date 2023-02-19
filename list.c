
#include "list.h"
#include "vm.h"
#include "gc.h"

nost_cons* nost_makeCons(nost_vm* vm, nost_val car, nost_val cdr) {
    nost_cons* cons = (nost_cons*)nost_allocObj(vm, NOST_OBJ_CONS, sizeof(nost_cons)); 
    cons->car = car;
    cons->cdr = cdr;
    return cons;
}

nost_val nost_list(struct nost_vm* vm, int cnt, nost_val* elems) {
    nost_gcPause(vm);
    nost_val res = nost_nil();
    for(int i = cnt - 1; i >= 0; i--) {
        res = nost_objVal((nost_obj*)nost_makeCons(vm, elems[i], res));
    } 
    nost_gcUnpause(vm);
    return res;
}

nost_val nost_car(struct nost_vm* vm, nost_cons* cons) {
    return cons->car;
}

nost_val nost_cdr(struct nost_vm* vm, nost_cons* cons) {
    return cons->cdr;
}

bool nost_nilTerminated(nost_vm* vm, nost_val list) {
    while(nost_isCons(list))
        list = nost_cdr(vm, nost_asCons(list));
    return nost_isNil(list);
}

int nost_len(nost_vm* vm, nost_val list) {
    int res = 0;
    while(nost_isCons(list)) {
        list = nost_cdr(vm, nost_asCons(list)); 
        res++;
    }
    return res;
}

nost_val nost_nth(struct nost_vm* vm, nost_val list, int n) {
    for(int i = 0; i < n; i++) {
        list = nost_cdr(vm, nost_asCons(list));
    }
    return nost_car(vm, nost_asCons(list));
}
