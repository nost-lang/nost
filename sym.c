
#include "sym.h"
#include "vm.h"

nost_val nost_makeSym(nost_vm* vm, const char* sym, size_t len) {
    if(len == 3 && memcmp(sym, "nil", 3) == 0)
        return nost_nil();
    nost_sym* symObj = (nost_sym*)nost_allocObj(vm, NOST_OBJ_SYM, sizeof(nost_sym));
    char* symStr = nost_alloc(vm, len + 1);
    memcpy(symStr, sym, len);
    symStr[len] = '\0';
    symObj->len = len;
    symObj->sym = symStr;
    return nost_objVal((nost_obj*)symObj); 
}

bool nost_symEq(nost_sym* a, nost_sym* b) {
    return strcmp(a->sym, b->sym) == 0;
}

bool nost_symIs(nost_val val, const char* str) {
    if(!nost_isSym(val))
        return false; 
    nost_sym* sym = nost_asSym(val);
    size_t strLen = strlen(str);
    return strLen == sym->len && memcmp(sym->sym, str, strLen) == 0;
}
