
#include "stdlib.h"
#include "embed.h"
#include "list.h"
#include "src.h"
#include <stdio.h> // TODO: printing should be nat fun from embedder

static nost_val car(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, nost_nil(), 1, (const char*[]){"Expected cons pair."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    if(!nost_isCons(args[0])) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Cannot take car of %s.", nost_typename(args[0]));
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    return nost_car(vm, nost_asCons(args[0]));
}

static nost_val cdr(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, nost_nil(), 1, (const char*[]){"Expected cons pair."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    if(!nost_isCons(args[0])) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Cannot take cdr of %s.", nost_typename(args[0]));
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    return nost_cdr(vm, nost_asCons(args[0]));
}

static nost_val cons(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 2) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, nost_nil(), 2, (const char*[]){"Expected value.", "Expected value."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    return nost_objVal((nost_obj*)nost_makeCons(vm, args[0], args[1]));
}

void nost_printVal(nost_vm* vm, nost_val val) {
    nost_str valStr;
    nost_initStr(vm, &valStr);
    nost_writeVal(vm, &valStr, val);
    printf("%s", valStr.str);
    nost_freeStr(vm, &valStr);
}

static nost_val printlnVal(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, nost_nil(), 1, (const char*[]){"Expected value to print."});
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    nost_printVal(vm, args[0]);
    printf("\n");
    return nost_nil();
} 

void nost_initStdlib(nost_vm* vm) {
    nost_addNatfn(vm, vm->rootCtx, "car", car);
    nost_addNatfn(vm, vm->rootCtx, "cdr", cdr);
    nost_addNatfn(vm, vm->rootCtx, "cons", cons);
    nost_addNatfn(vm, vm->rootCtx, "pr", printlnVal);
}
