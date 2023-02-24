
#include "eval.h"
#include "list.h"
#include "gc.h"
#include "fn.h"
#include <stdio.h>

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val expr, int depth);

static nost_val callFn(nost_vm* vm, nost_fiber* fiber, nost_fn* fn, nost_val args, int argLen, int depth) {
    nost_val argArr[argLen];
    for(int i = 0; i < argLen; i++) {
        argArr[i] = nost_car(vm, nost_asCons(args));
        args = nost_cdr(vm, nost_asCons(args));
    }
    nost_val evaluatedArgs = nost_nil();
    for(int i = argLen - 1; i >= 0; i--) {
        nost_val nextArg = eval(vm, fiber, argArr[i], depth + 1);
        if(fiber->hadError)
            return nost_nil();
        nost_gcPause(vm);
        nost_val newEvaluatedArgs = nost_objVal((nost_obj*)nost_makeCons(vm, nextArg, evaluatedArgs));
        nost_blessVal(vm, newEvaluatedArgs);
        nost_unblessVal(vm, evaluatedArgs);
        nost_gcUnpause(vm);
        evaluatedArgs = newEvaluatedArgs;
    }
    nost_pushFrame(vm, fiber, fn->closureCtx);
    nost_addDynvar(vm, fiber, fn->argName);
    nost_setVar(fiber, fn->argName, evaluatedArgs);
    nost_val res = eval(vm, fiber, fn->body, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    nost_popFrame(vm, fiber);

    return res;
}

static nost_val callMacro(nost_vm* vm, nost_fiber* fiber, nost_fn* fn, nost_val args, int depth) {
    nost_pushFrame(vm, fiber, fn->closureCtx);
    nost_addDynvar(vm, fiber, fn->argName);
    nost_setVar(fiber, fn->argName, args);
    nost_val res = eval(vm, fiber, fn->body, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    nost_popFrame(vm, fiber);
    return eval(vm, fiber, res, depth + 1);
}

static nost_val call(nost_vm* vm, nost_fiber* fiber, nost_val fn, nost_val args, int depth) {
    int argLen = nost_len(vm, args);
    if(nost_symIs(fn, "quote")) {
        if(argLen != 1) {
            nost_error err;
            nost_initError(&err, "Quote takes 1 arg");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        return nost_car(vm, nost_asCons(args));
    }
    if(nost_symIs(fn, "var")) {
        if(argLen != 2) {
            nost_error err;
            nost_initError(&err, "Invalid # of args for var.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_val nameVal = nost_nth(vm, args, 0);
        nost_val val = eval(vm, fiber, nost_nth(vm, args, 1), depth + 1); 
        if(fiber->hadError)
            return nost_nil();
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
    if(nost_symIs(fn, "progn")) {
        nost_val res = nost_nil();
        while(!nost_isNil(args)) {
            nost_cons* argsCons = nost_asCons(args);
            nost_unblessVal(vm, res); // TODO: this is slow.
            res = eval(vm, fiber, nost_car(vm, argsCons), depth + 1);
            if(fiber->hadError)
                return nost_nil();
            nost_blessVal(vm, res);
            args = nost_cdr(vm, argsCons);
        }
        nost_unblessVal(vm, res);
        return res;
    }
    if(nost_symIs(fn, "scope")) {
        if(argLen != 1) {
            nost_error err;
            nost_initError(&err, "Expected only 1 subexpr.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_cons* argsCons = nost_asCons(args);
        nost_pushCtx(vm, fiber);
        nost_val res = eval(vm, fiber, nost_car(vm, argsCons), depth + 1);
        if(fiber->hadError)
            return nost_nil();
        nost_popCtx(fiber);
        return res;
    }
    if(nost_symIs(fn, "if")) {
        if(argLen != 3 && argLen != 2) {
            nost_error err;
            nost_initError(&err, "Expected 2 or 3 args for if.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_val cond = eval(vm, fiber, nost_nth(vm, args, 0), depth + 1);
        if(fiber->hadError)
            return nost_nil();

        nost_val res;
        if(nost_truthy(cond)) {
            nost_pushCtx(vm, fiber);
            res = eval(vm, fiber, nost_nth(vm, args, 1), depth + 1);
            nost_popCtx(fiber);
        } else {
            if(argLen == 2) {
                nost_error err;
                nost_initError(&err, "If does not have false branch.");
                nost_rtError(fiber, err);
                return nost_nil();
            }
            nost_pushCtx(vm, fiber); 
            res = eval(vm, fiber, nost_nth(vm, args, 2), depth + 1);
            nost_popCtx(fiber);
        }
        if(fiber->hadError)
            return nost_nil();
        return res;
    }
    if(nost_symIs(fn, "lambda") || nost_symIs(fn, "lamac")) {
        if(argLen != 2) {
            nost_error err;
            nost_initError(&err, "Expected argname and body.");
            nost_rtError(fiber, err);
            return nost_nil();
        }
        nost_val argName = nost_nth(vm, args, 0);
        nost_val body = nost_nth(vm, args, 1);
        if(!nost_isSym(argName)) {
            nost_error err;
            nost_initError(&err, "Argname must be a sym.");
            nost_rtError(fiber, err);
            return nost_nil();
        }

        if(nost_symIs(fn, "lambda")) {
            nost_fn* newFn = nost_makeFn(vm, nost_asSym(argName), body, nost_currCtx(fiber));
            return nost_objVal((nost_obj*)newFn);
        } else {
            nost_fn* newMacro = nost_makeMacro(vm, nost_asSym(argName), body, nost_currCtx(fiber));
            return nost_objVal((nost_obj*)newMacro);
        }
    }

    nost_val fnVal = eval(vm, fiber, fn, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    if(nost_isFn(fnVal)) {
        nost_fn* fn = nost_asFn(fnVal);
        if(!fn->macro)
            return callFn(vm, fiber, fn, args, argLen, depth);
        else
            return callMacro(vm, fiber, fn, args, depth);
    }
    if(nost_isNatfn(fnVal)) {
        nost_natfn* natfn = nost_asNatfn(fnVal);
        nost_val argVals[argLen];
        for(int i = 0; i < argLen; i++) {
            nost_cons* argCons = nost_asCons(args);
            nost_val argVal = eval(vm, fiber, nost_car(vm, argCons), depth + 1);
            args = nost_cdr(vm, argCons);
            argVals[i] = argVal; 
            nost_blessVal(vm, argVal);
        }
        nost_val res = natfn->fn(vm, fiber, argLen, argVals);    
        for(int i = 0; i < argLen; i++) {
            nost_unblessVal(vm, argVals[i]);
        }
        return res;
    }

    nost_error err;
    nost_initError(&err, "Cannot call this.");
    nost_rtError(fiber, err);
    return nost_nil(); 
}

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val expr, int depth) {
    if(depth >= 4096) {
        nost_error err;
        nost_initError(&err, "Eval depth exceeded.");
        nost_rtError(fiber, err);
        return nost_nil(); 
    }
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
        return call(vm, fiber, fn, nost_cdr(vm, cons), depth); 
    }

    nost_error err;
    nost_initError(&err, "Cannot evalutate this.");
    nost_rtError(fiber, err);
    return nost_nil();
} 

nost_val nost_eval(nost_vm* vm, nost_fiber* fiber, nost_val expr) {
    return eval(vm, fiber, expr, 0);
}
