
#include "eval.h"
#include "list.h"
#include "gc.h"
#include "fn.h"
#include "pkg.h"
#include "src.h"
#include "stdlib.h"
#include <stdio.h>

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val expr, int depth);

static nost_val callFn(nost_vm* vm, nost_fiber* fiber, nost_val code, nost_fn* fn, nost_val args, int argLen, int depth) {
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
    nost_pushFrame(vm, fiber, fn->closureCtx, fn->pkg, code);
    nost_addDynvar(vm, fiber, fn->argName);
    nost_setVar(fiber, fn->argName, evaluatedArgs);
    nost_val res = eval(vm, fiber, fn->body, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    nost_popFrame(vm, fiber);

    return res;
}

static nost_val callMacro(nost_vm* vm, nost_fiber* fiber, nost_val code, nost_fn* fn, nost_val args, int depth) {
    nost_pushFrame(vm, fiber, fn->closureCtx, fn->pkg, code);
    nost_addDynvar(vm, fiber, fn->argName);
    nost_setVar(fiber, fn->argName, args);
    nost_val res = eval(vm, fiber, fn->body, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    nost_popFrame(vm, fiber);
    return eval(vm, fiber, res, depth + 1);
}

static nost_val evalQuote(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args) {
    if(argLen != 1) {
        nost_error err;
        nost_initError(vm, &err);
        if(argLen == 0) {
            nost_addMessage(vm, &err, "Need something to quote.");
            nost_addValEndRef(vm, &err, code);
        } else {
            nost_addMessage(vm, &err, "Quote takes only 1 argument.");
            nost_addListRef(vm, &err, nost_unwrap(code), 2, argLen);
        }
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    return nost_car(vm, nost_asCons(args));
}

static nost_val evalVar(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args, int depth) {

    if(argLen != 2) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 2, (const char*[]){"Expected variable name.", "Expected initial value."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
   
    nost_val nameValWrapped = nost_nth(vm, args, 0);
    nost_val nameVal = nost_unwrap(nameValWrapped);
    nost_val val = eval(vm, fiber, nost_nth(vm, args, 1), depth + 1); 
    if(fiber->hadError)
        return nost_nil();
    if(!nost_isSym(nameVal)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Variable name must be a symbol.");
        nost_addValRef(vm, &err, nameValWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    nost_sym* name = nost_asSym(nameVal);
    if(!nost_addDynvarWithDecl(vm, fiber, name, code)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Variable redeclaration.");
        nost_addValRef(vm, &err, nameValWrapped);
        nost_val decl = nost_getVarDecl(vm, fiber, name);
        if(!nost_isNil(decl)) {
            nost_addMessage(vm, &err, "Note: initial declaration here:");
            nost_addValRef(vm, &err, decl);
        }
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    nost_setVar(fiber, name, val);
    return val;
}

static nost_val evalProgn(nost_vm* vm, nost_fiber* fiber, nost_val args, int depth) {
    nost_val res = nost_nil();
    while(!nost_isNil(args)) {
        nost_cons* argsCons = nost_asCons(args);
        nost_unblessVal(vm, res); // TODO: this is dumb.
        res = eval(vm, fiber, nost_car(vm, argsCons), depth + 1);
        if(fiber->hadError)
            return nost_nil();
        nost_blessVal(vm, res);
        args = nost_cdr(vm, argsCons);
    }
    nost_unblessVal(vm, res);
    return res;
}

static nost_val evalScope(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args, int depth) {
    if(argLen != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 1, (const char*[]){"Scope needs to have a subexpression."});
        nost_rtError(vm, fiber, err);
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

static nost_val evalIf(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args, int depth) {
    if(argLen != 3 && argLen != 2) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 3, (const char*[]){"Expected condition.", "Expected true branch.", NULL});
        nost_rtError(vm, fiber, err);
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

static nost_val evalLambda(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args) {
    if(argLen != 2) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 2, (const char*[]){"Expected argname.", "Expected function body."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    nost_val argNameWrapped = nost_nth(vm, args, 0);
    nost_val argName = nost_unwrap(argNameWrapped); 
    nost_val body = nost_nth(vm, args, 1);
    if(!nost_isSym(argName)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Argname must be a symbol.");
        nost_addValRef(vm, &err, argNameWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }

    nost_fn* fn = nost_makeFn(vm, nost_asSym(argName), body, nost_currCtx(fiber), nost_currFrame(fiber)->pkg);
    return nost_objVal((nost_obj*)fn);
}

static nost_val evalLamac(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args) {
    if(argLen != 2) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 2, (const char*[]){"Expected argname.", "Expected macro body."});
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }
    nost_val argNameWrapped = nost_nth(vm, args, 0);
    nost_val argName = nost_unwrap(argNameWrapped); 
    nost_val body = nost_nth(vm, args, 1);
    if(!nost_isSym(argName)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Argname must be a symbol.");
        nost_addValRef(vm, &err, argNameWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil();
    }

    nost_fn* macro = nost_makeMacro(vm, nost_asSym(argName), body, nost_currCtx(fiber), nost_currFrame(fiber)->pkg);
    return nost_objVal((nost_obj*)macro);
}

static nost_val evalImport(nost_vm* vm, nost_fiber* fiber, nost_val code, int argLen, nost_val args) {
    if(argLen != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 1, (const char*[]){"Expected package name."});
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    nost_val pkgNameWrapped = nost_nth(vm, args, 0);
    nost_val pkgName = nost_unwrap(pkgNameWrapped); 
    if(!nost_isSym(pkgName)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Package name must be a symbol.");
        nost_addValRef(vm, &err, pkgNameWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    nost_sym* pkgNameSym = nost_asSym(pkgName);
    nost_pkg* pkg = nost_loadPkg(vm, fiber, pkgNameSym->sym, nost_currFrame(fiber)->pkg, code);
    if(fiber->hadError)
        return nost_nil();
    if(pkg == NULL) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Package '%s' not found.", pkgNameSym->sym);
        nost_addValRef(vm, &err, pkgNameWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    return nost_objVal((nost_obj*)pkg);
}

static nost_val callPkg(nost_vm* vm, nost_fiber* fiber, nost_val code, nost_pkg* pkg, int argLen, nost_val args, int depth) {
    if(argLen != 1) {
        nost_error err;
        nost_initError(vm, &err);
        nost_doArgCntErrors(vm, &err, code, 1, (const char*[]){"Expected package member name."});
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    nost_val memNameValWrapped = eval(vm, fiber, nost_nth(vm, args, 0), depth + 1);
    nost_val memNameVal = nost_unwrap(memNameValWrapped); 
    if(fiber->hadError)
        return nost_nil();
    if(!nost_isSym(memNameVal)) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Member name must be a sym.");
        nost_addValRef(vm, &err, memNameValWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    nost_sym* memName = nost_asSym(memNameVal);
    nost_optVal member = nost_getVarInCtx(pkg->ctx, memName);
    if(member.nil) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Member '%s' not found in package '%s'.", memName->sym, pkg->name);
        nost_addValRef(vm, &err, memNameValWrapped);
        nost_rtError(vm, fiber, err);
        return nost_nil(); 
    }
    return member.val;
}

static nost_val callNatfn(nost_vm* vm, nost_fiber* fiber, nost_val code, nost_natfn* natfn, int argLen, nost_val args, int depth) {
    nost_val argVals[argLen];
    for(int i = 0; i < argLen; i++) {
        nost_cons* argCons = nost_asCons(args);
        nost_val argVal = eval(vm, fiber, nost_car(vm, argCons), depth + 1);
        if(fiber->hadError)
            return nost_nil(); // TODO: this leaks blessed vals. REALLY BAD. FIX! 
        args = nost_cdr(vm, argCons);
        argVals[i] = argVal; 
        nost_blessVal(vm, argVal);
    }
    nost_pushFrame(vm, fiber, NULL, NULL, code);
    nost_val res = natfn->fn(vm, fiber, argLen, argVals);    
    for(int i = 0; i < argLen; i++) {
        nost_unblessVal(vm, argVals[i]);
    }
    nost_popFrame(vm, fiber);
    return res;
}

static nost_val call(nost_vm* vm, nost_fiber* fiber, nost_val code, nost_val fn, nost_val args, int depth) {
    int argLen = nost_len(vm, args);
    if(nost_symIs(fn, "quote"))
        return evalQuote(vm, fiber, code, argLen, args);
    if(nost_symIs(fn, "var"))
        return evalVar(vm, fiber, code, argLen, args, depth);
    if(nost_symIs(fn, "progn"))
        return evalProgn(vm, fiber, args, depth);
    if(nost_symIs(fn, "scope"))
        return evalScope(vm, fiber, code, argLen, args, depth);
    if(nost_symIs(fn, "if"))
        return evalIf(vm, fiber, code, argLen, args, depth);
    if(nost_symIs(fn, "lambda"))
        return evalLambda(vm, fiber, code, argLen, args); 
    if(nost_symIs(fn, "lamac"))
        return evalLamac(vm, fiber, code, argLen, args); 
    if(nost_symIs(fn, "import"))
        return evalImport(vm, fiber, code, argLen, args);

    nost_val fnVal = eval(vm, fiber, fn, depth + 1);
    if(fiber->hadError)
        return nost_nil();
    if(nost_isFn(fnVal)) {
        nost_fn* fn = nost_asFn(fnVal);
        if(!fn->macro)
            return callFn(vm, fiber, code, fn, args, argLen, depth);
        else
            return callMacro(vm, fiber, code, fn, args, depth);
    }
    if(nost_isPkg(fnVal))
        return callPkg(vm, fiber, code, nost_asPkg(fnVal), argLen, args, depth);
    if(nost_isNatfn(fnVal))
        return callNatfn(vm, fiber, code, nost_asNatfn(fnVal), argLen, args, depth);

    nost_error err;
    nost_initError(vm, &err);
    nost_addMessage(vm, &err, "Cannot call %s.", nost_typename(fnVal));
    nost_addListRef(vm, &err, nost_unwrap(code), 0, 0);
    nost_rtError(vm, fiber, err);
    return nost_nil(); 
}

static nost_val eval(nost_vm* vm, nost_fiber* fiber, nost_val code, int depth) {

    nost_val expr = nost_unwrap(code);

    if(depth >= 4096) {
        nost_error err;
        nost_initError(vm, &err);
        nost_addMessage(vm, &err, "Eval depth exceeded.");
        nost_addValRef(vm, &err, code);
        nost_rtError(vm, fiber, err);
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
            nost_initError(vm, &err);
            nost_addMessage(vm, &err, "Variable '%s' doesn't exist.", name->sym);
            nost_addValRef(vm, &err, code);
            nost_rtError(vm, fiber, err);
            return nost_nil(); 
        }
        return varVal.val;
    } else if(nost_isCons(expr)) {
        if(!nost_nilTerminated(vm, expr)) {
            nost_error err;
            nost_initError(vm, &err);
            nost_addMessage(vm, &err, "S-Expr must be nil-terminated.");
            nost_addValRef(vm, &err, code);
            nost_rtError(vm, fiber, err);
            return nost_nil();
        }
        nost_cons* cons = nost_asCons(expr);
        nost_val fn = nost_unwrap(nost_car(vm, cons));
        return call(vm, fiber, code, fn, nost_cdr(vm, cons), depth); 
    }

    nost_error err;
    nost_initError(vm, &err);
    nost_addMessage(vm, &err, "Cannot evalutate this.");
    nost_addValRef(vm, &err, code);
    nost_rtError(vm, fiber, err);
    return nost_nil();
} 

nost_val nost_eval(nost_vm* vm, nost_fiber* fiber, nost_val expr) {
    return eval(vm, fiber, expr, 0);
}
