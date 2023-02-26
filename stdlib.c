
#include "stdlib.h"
#include "embed.h"
#include "list.h"
#include <stdio.h> // TODO: printing should be nat fun from embedder

static nost_val car(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(&err, "car takes 1 arg.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    if(!nost_isCons(args[0])) {
        nost_error err;
        nost_initError(&err, "car takes cons pair.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    return nost_car(vm, nost_asCons(args[0]));
}

static nost_val cdr(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(&err, "cdr takes 1 arg.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    if(!nost_isCons(args[0])) {
        nost_error err;
        nost_initError(&err, "cdr takes cons pair.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    return nost_cdr(vm, nost_asCons(args[0]));
}

static nost_val cons(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 2) {
        nost_error err;
        nost_initError(&err, "Cons takes 2 args.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    return nost_objVal((nost_obj*)nost_makeCons(vm, args[0], args[1]));
}

void printVal(nost_vm* vm, nost_val val) {
    if(nost_isNil(val)) {
        printf("nil");
    } else if(nost_isNum(val)) {
        printf("%g", nost_asNum(val));
    } else if(nost_isSym(val)) {
        printf("%s", nost_asSym(val)->sym);
    } else if(nost_isCons(val)) {
        nost_cons* cons = nost_asCons(val);
        nost_val car = nost_car(vm, cons); 
        nost_val cdr = nost_cdr(vm, cons);
        printf("(");
        while(true) {
            printVal(vm, car);
            if(nost_isNil(cdr)) {
                printf(")");
                break;
            } else if(nost_isCons(cdr)) {
                printf(" ");
                cons = nost_asCons(cdr);
                car = nost_car(vm, cons); 
                cdr = nost_cdr(vm, cons);
            } else {
                printf(" . ");
                printVal(vm, cdr);
                printf(")");
                break;
            }
        } 
    } else if(nost_isFn(val)) {
        printf("<fn>");
    } else if(nost_isNatfn(val)) {
        printf("<natfn>");
    } else if(nost_isPkg(val)) {
        printf("<pkg>");
    } else {
        printf("UNKNOWN VALUE");
    }
}

nost_val printlnVal(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 1) {
        nost_error err;
        nost_initError(&err, "print takes 1 args.");
        nost_rtError(fiber, err);
        return nost_nil(); 
    }
    printVal(vm, args[0]);
    printf("\n");
    return nost_nil();
} 

void nost_initStdlib(nost_vm* vm) {
    nost_addNatfn(vm, vm->rootCtx, "car", car);
    nost_addNatfn(vm, vm->rootCtx, "cdr", cdr);
    nost_addNatfn(vm, vm->rootCtx, "cons", cons);
    nost_addNatfn(vm, vm->rootCtx, "pr", printlnVal);
}
