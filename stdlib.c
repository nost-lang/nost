
#include "stdlib.h"
#include "fiber.h"
#include "list.h"
#include "embed.h"
#include "gc.h"
#include "util.h"

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

static nost_val eq(nost_vm* vm, nost_ref fiber, int nArgs, nost_val* args) {
    if(nArgs != 2) {
        nost_natFnError(vm, fiber, "Must compare two values.");
        return nost_nilVal();
    }
    bool eq = nost_eq(args[0], args[1]);
    return eq ? vm->t : nost_nilVal();
}

void nost_initStdlib(nost_vm* vm) {
    vm->stdlibCtx = nost_makeCtx(vm, nost_nilVal());
    nost_ref ctx = NOST_PUSH_BLESSING(vm, vm->stdlibCtx);
    
    nost_addNatFn(vm, ctx, "car", car);
    nost_addNatFn(vm, ctx, "cdr", cdr);

    nost_addNatFn(vm, ctx, "+", plus);
    nost_addNatFn(vm, ctx, "-", minus);
    nost_addNatFn(vm, ctx, "eq", eq);

    NOST_POP_BLESSING(vm, ctx);
}
