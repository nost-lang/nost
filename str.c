
#include "str.h"
#include "gc.h"
#include "list.h"

void nost_initStr(nost_str* str) {
    str->str = NULL;
    str->len = 0;
    str->cap = 0;
}

void nost_freeStr(nost_vm* vm, nost_str* str) {
    NOST_GC_FREE(vm, str->str, str->cap);
}

void nost_writeStr(nost_vm* vm, nost_str* str, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt); 
    va_list argsCopy1;
    va_copy(argsCopy1, args);
    
    size_t len = vsnprintf(NULL, 0, fmt, args);
    size_t oldLen = str->len;
    str->len += len;
    size_t initCap = str->cap;
    if(str->cap == 0)
        str->cap = 8;
    while(str->len >= str->cap - 1) {
        str->cap *= 2;
    }
    if(initCap != str->cap)
        str->str = NOST_GC_REALLOC(vm, str->str, initCap, str->cap);
    vsnprintf(str->str + oldLen, len + 1, fmt, argsCopy1);

    va_end(args);
    va_end(argsCopy1); 
}

void nost_writeVal(nost_vm* vm, nost_str* str, nost_val val) {
    NOST_ASSERT(!nost_isNone(val), "None should not be user visible.");
    if(nost_isNil(val)) {
        nost_writeStr(vm, str, "nil");
        return;
    }    
    if(nost_isNum(val)) {
        nost_writeStr(vm, str, "%g", nost_asNum(val));
        return;
    }
    if(nost_isCons(val)) {
        nost_ref ref = NOST_PUSH_BLESSING(vm, val);
        nost_writeStr(vm, str, "(");
        while(true) {
            nost_writeVal(vm, str, nost_refCar(vm, ref));
            if(nost_isNil(nost_refCdr(vm, ref))) {
                nost_writeStr(vm, str, ")");
                break;
            } else if(nost_isCons(nost_refCdr(vm, ref))) {
                nost_writeStr(vm, str, " ");
            } else {
                nost_writeStr(vm, str, " . ");
            }
            nost_setRef(vm, ref, nost_refCdr(vm, ref));
        }
        NOST_POP_BLESSING(vm, ref);
        return;
    }
    if(nost_isObj(val)) {
        nost_writeStr(vm, str, "<%s>", nost_typename(val));
        return;
    } 
}
