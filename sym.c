
#include "sym.h"

nost_val nost_makeSymWithLen(nost_vm* vm, int len) {
    nost_sym* sym = (nost_sym*)nost_allocObj(vm, NOST_OBJ_SYM, sizeof(nost_sym) + len + 1); 
    sym->len = len;
    sym->sym[0] = '\0';
    return nost_objVal((nost_obj*)sym);
}

void nost_initSym(nost_val symVal, const char* start) {
    nost_sym* sym = nost_asSym(symVal); 
    memcpy(sym->sym, start, sym->len);
    sym->sym[sym->len] = '\0';
}

bool nost_symEq(nost_val aVal, nost_val bVal) {
    nost_sym* a = nost_asSym(aVal);
    nost_sym* b = nost_asSym(bVal);
    if(a->len != b->len)
        return false;
    return memcmp(a->sym, b->sym, a->len) == 0;
}
