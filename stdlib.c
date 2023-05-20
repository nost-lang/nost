
#include "stdlib.h"
#include "fiber.h"
#include "list.h"
#include "embed.h"
#include "gc.h"
#include "util.h"
#include "sym.h"

static nost_val car(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs != 1) {
        nost_natFnError(vm, fiber, "car takes one argument.");
        return nost_nilVal();
    }
    if(!nost_isConslike(args[0])) {
        nost_natFnError(vm, fiber, "Cannot take car of %s.", nost_typename(args[0]));
        return nost_nilVal();
    } 
    return nost_car(args[0]);
}

static nost_val cdr(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs != 1) {
        nost_natFnError(vm, fiber, "cdr takes one argument.");
        return nost_nilVal();
    }
    if(!nost_isConslike(args[0])) {
        nost_natFnError(vm, fiber, "Cannot take cdr of %s.", nost_typename(args[0]));
        return nost_nilVal();
    } 
    return nost_cdr(args[0]);
}

static nost_val cons(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs != 2) {
        nost_natFnError(vm, fiber, "cons takes two arguments.");
        return nost_nilVal();
    }
    nost_val cons = nost_makeCons(vm);
    nost_initCons(cons, args[0], args[1]);
    return cons;
}

static nost_val plus(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    double res = 0.0;
    for(int i = 0; i < nArgs; i++) {
        if(!nost_isNum(args[i])) {
            nost_natFnError(vm, fiber, "%d%s argument is not a number.", i + 1, nost_ordinalSuffix(i + 1));
            return nost_nilVal();
        }
        res += nost_asNum(args[i]);
    }
    return nost_numVal(res);
}

static nost_val minus(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    double res = 0.0;
    for(int i = 0; i < nArgs; i++) {
        if(!nost_isNum(args[i])) {
            nost_natFnError(vm, fiber, "%d%s argument is not a number.", i + 1, nost_ordinalSuffix(i + 1));
            return nost_nilVal();
        }
        if(i == 0)
            res += nost_asNum(args[i]);
        else
            res -= nost_asNum(args[i]);
    }
    return nost_numVal(res);
}

static nost_val times(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    double res = 1.0;
    for(int i = 0; i < nArgs; i++) {
        if(!nost_isNum(args[i])) {
            nost_natFnError(vm, fiber, "%d%s argument is not a number.", i + 1, nost_ordinalSuffix(i + 1));
            return nost_nilVal();
        }
        res *= nost_asNum(args[i]);
    }
    return nost_numVal(res);
}

static nost_val divide(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    double res = 1.0;
    for(int i = 0; i < nArgs; i++) {
        if(!nost_isNum(args[i])) {
            nost_natFnError(vm, fiber, "%d%s argument is not a number.", i + 1, nost_ordinalSuffix(i + 1));
            return nost_nilVal();
        }
        if(i == 0)
            res *= nost_asNum(args[i]);
        else
            res /= nost_asNum(args[i]);
    }
    return nost_numVal(res);
}

static nost_val eq(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs != 2) {
        nost_natFnError(vm, fiber, "Must compare two values.");
        return nost_nilVal();
    }
    bool eq = nost_eq(args[0], args[1]);
    return eq ? vm->t : nost_nilVal();
}

static nost_val gensym(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs > 1) {
        nost_natFnError(vm, fiber, "Unexpected arguments.");
        return nost_nilVal();
    }
    vm->gensymIdx++;
    nost_val sym = nost_nilVal();
    if(nArgs == 0) {
        char res[64];
        sprintf(res, "✨%d", vm->gensymIdx);
        sym = nost_makeSymWithLen(vm, strlen(res)); 
        nost_initSym(sym, res);
    } else {
        if(!nost_isSym(args[0])) {
            nost_natFnError(vm, fiber, "Gensym description must be a symbol.");
            return nost_nilVal();
        }
        char res[64 + nost_asSym(args[0])->len];
        sprintf(res, "✨%d-%s", vm->gensymIdx, nost_asSym(args[0])->sym);
        sym = nost_makeSymWithLen(vm, strlen(res)); 
        nost_initSym(sym, res);
    }
    return sym;
}

void nost_initStdlib(nost_vm* vm) {
    vm->stdlibCtx = nost_makeCtx(vm, nost_nilVal());
    nost_ref ctx = NOST_PUSH_BLESSING(vm, vm->stdlibCtx);
    
    nost_addNatFn(vm, ctx, "car", car);
    nost_addNatFn(vm, ctx, "cdr", cdr);
    nost_addNatFn(vm, ctx, "cons", cons);

    nost_addNatFn(vm, ctx, "+", plus);
    nost_addNatFn(vm, ctx, "-", minus);
    nost_addNatFn(vm, ctx, "*", times);
    nost_addNatFn(vm, ctx, "/", divide);

    nost_addNatFn(vm, ctx, "eq", eq);

    nost_addNatFn(vm, ctx, "gensym", gensym);

    NOST_POP_BLESSING(vm, ctx);
}
