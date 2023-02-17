
#include "eval.h"
#include "list.h"
#include "gc.h"

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val expr);

static nost_val call(nost_vm* vm, nost_fiber* fiber, nost_val fn, nost_val args) {
    int argLen = nost_len(vm, args);
    if(nost_symIs(fn, "var")) {
        if(argLen != 2) {
            nost_error err;
            nost_initError(&err, "Invalid # of args for var.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_val nameVal = nost_nth(vm, args, 0);
        nost_val val = eval(vm, fiber, nost_nth(vm, args, 1)); 
        if(!nost_isSym(nameVal)) {
            nost_error err;
            nost_initError(&err, "Variable name must be a symbol.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_sym* name = nost_asSym(nameVal);
        if(!nost_addDynvar(vm, fiber, name)) {
            nost_error err;
            nost_initError(&err, "Variable redeclaration.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_setVar(fiber, name, val);
        return val;
    }
    nost_error err;
    nost_initError(&err, "Cannot call this.");
    nost_rtError(fiber, err);
    return nost_nil(); 
}

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val expr) {
    if(nost_isNil(expr)) {
        return nost_nil();
    } else if(nost_isNum(expr)) {
        return expr; 
    } else if(nost_isSym(expr)) {
        nost_sym* name = nost_asSym(expr);
        nost_optVal varVal = nost_getVar(fiber, name);
        if(varVal.nil) {
            nost_error err;
            nost_initError(&err, "Variable doesn't exist.");
            nost_rtError(fiber, err);
            return nost_nil(); 
        }
        return varVal.val;
    } else if(nost_isCons(expr)) {
        if(!nost_nilTerminated(vm, expr)) {
            nost_error err;
            nost_initError(&err, "S-Expr must be nil-terminated.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_cons* cons = nost_asCons(expr);
        nost_val fn = nost_car(vm, cons);
        return call(vm, fiber, fn, nost_cdr(vm, cons)); 
    }

    nost_error err;
    nost_initError(&err, "Cannot evalutate this.");
    nost_rtError(fiber, err);
    return nost_nil();
} 

nost_val nost_eval(nost_vm* vm, nost_fiber* fiber, nost_val expr) {
    return eval(vm, fiber, expr);
}
