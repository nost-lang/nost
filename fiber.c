
#include "fiber.h"
#include "gc.h"
#include "bytecode.h"
#include "sym.h"
#include "src.h"
#include "fn.h"

nost_val nost_makeCtx(nost_vm* vm, nost_val parent) {
    nost_ctx* ctx = (nost_ctx*)nost_allocObj(vm, NOST_OBJ_CTX, sizeof(nost_ctx));
    nost_initGCDynarr(&ctx->vars);
    ctx->parent = parent; 
    return nost_objVal((nost_obj*)ctx);
}

void nost_addDynvar(nost_vm* vm, nost_ref ctx, nost_val nameVal, nost_val valVal) {
    nost_ref name = nost_pushBlessing(vm, nameVal);
    nost_ref val = nost_pushBlessing(vm, valVal);

    nost_gcDynarr(nost_dynvar) newVars;
    nost_dynvar var;
    var.name = nost_nilVal();
    var.val = nost_nilVal();
    nost_gcPushDynarr(vm, &nost_refAsCtx(vm, ctx)->vars, var, &newVars);
    nost_writeBarrier(vm, nost_getRef(vm, ctx), nost_getRef(vm, name));
    nost_writeBarrier(vm, nost_getRef(vm, ctx), nost_getRef(vm, val));
    newVars.vals[newVars.cnt - 1].name = nost_getRef(vm, name);
    newVars.vals[newVars.cnt - 1].val = nost_getRef(vm, val);
    nost_moveGCDynarr(&newVars, &nost_refAsCtx(vm, ctx)->vars);

    nost_popBlessing(vm);
    nost_popBlessing(vm);
}

static bool varInCtx(nost_val name, nost_val ctxVal) {
    nost_ctx* ctx = nost_asCtx(ctxVal);
    for(int i = 0; i < ctx->vars.cnt; i++) {
        if(nost_symEq(ctx->vars.vals[i].name, name)) {
            return true;
        }
    }
    return false;
}

nost_val nost_getVar(bool* valid, nost_val ctxVal, nost_val name) {
    nost_ctx* ctx = nost_asCtx(ctxVal);
    for(int i = 0; i < ctx->vars.cnt; i++) {
        if(nost_symEq(ctx->vars.vals[i].name, name)) {
            *valid = true;
            return ctx->vars.vals[i].val; 
        }
    }
    if(nost_isNil(ctx->parent)) {
        *valid = false;
        return nost_nilVal();
    }
    return nost_getVar(valid, ctx->parent, name);
}

// TODO: move to dedicated stldib module
static nost_val add(nost_vm* vm, int nArgs, nost_val* args) {
    (void)vm;
    double sum = 0;
    for(int i = 0; i < nArgs; i++) {
        sum += nost_asNum(args[i]);
    }
    return nost_numVal(sum);
}

nost_val nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    fiber->bytecode = nost_nilVal();
    fiber->hadError = false;
    nost_initGCDynarr(&fiber->stack);
    nost_ref ref = nost_pushBlessing(vm, nost_objVal((nost_obj*)fiber));

    nost_val ctx = nost_makeCtx(vm, nost_nilVal());
    nost_ref ctxRef = nost_pushBlessing(vm, ctx);
    nost_val sym = nost_makeSymWithLen(vm, 1);
    nost_initSym(sym, "+");
    nost_ref symRef = nost_pushBlessing(vm, sym);
    nost_val fn = nost_makeNatFn(vm, add); 
    nost_addDynvar(vm, ctxRef, nost_getRef(vm, symRef), fn);
    nost_refAsFiber(vm, ref)->ctx = nost_getRef(vm, ctxRef); 

    nost_val res = nost_objVal(nost_refAsObj(vm, ref));
    nost_popBlessing(vm);
    return res;
}

static void push(nost_vm* vm, nost_ref fiber, nost_val val) {
    nost_ref ref = nost_pushBlessing(vm, val);
    nost_gcDynarr(nost_val) oldStack;
    nost_moveGCDynarr(&nost_refAsFiber(vm, fiber)->stack, &oldStack);
    nost_gcDynarr(nost_val) newStack;
    nost_gcPushDynarr(vm, &oldStack, nost_nilVal(), &newStack);
    nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_getRef(vm, ref));
    newStack.vals[newStack.cnt - 1] = nost_getRef(vm, ref);
    nost_moveGCDynarr(&newStack, &nost_refAsFiber(vm, fiber)->stack);
    nost_popBlessing(vm);
}

static nost_val pop(nost_vm* vm, nost_ref fiberRef) {
    nost_fiber* fiber = nost_refAsFiber(vm, fiberRef);
    NOST_ASSERT(fiber->stack.cnt > 0, "Cannot pop empty stack.")
    nost_val res = fiber->stack.vals[fiber->stack.cnt - 1]; 
    nost_popDynarr(vm, &fiber->stack);
    return res;
}

static nost_val peek(nost_vm* vm, nost_ref fiberRef) {
    nost_fiber* fiber = nost_refAsFiber(vm, fiberRef);
    NOST_ASSERT(fiber->stack.cnt > 0, "Cannot peek empty stack.")
    nost_val res = fiber->stack.vals[fiber->stack.cnt - 1]; 
    return res;
}

static void error(nost_vm* vm, nost_ref fiber, int ip, nost_ref bytecode) {
    nost_refAsFiber(vm, fiber)->hadError = true;
    nost_initError(&nost_refAsFiber(vm, fiber)->err);
    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Runtime Error.");
    // TODO: add full stack trace here
    bool found = false;
    for(int i = 0; i < nost_refAsBytecode(vm, bytecode)->errors.cnt; i++) {
        if(nost_refAsBytecode(vm, bytecode)->errors.vals[i].ip == ip) {
            nost_addValRef(vm, &nost_refAsFiber(vm, fiber)->err, nost_refAsBytecode(vm, bytecode)->errors.vals[i].src);
            found = true;
            break;
        }
    }
    NOST_ASSERT(found, "Specified IP not found in bytecode error points.");
}

nost_val nost_execBytecode(nost_vm* vm, nost_val fiberVal, nost_val bytecodeVal) {
    nost_ref fiber = nost_pushBlessing(vm, fiberVal);
    nost_ref bytecode = nost_pushBlessing(vm, bytecodeVal);

    nost_refAsFiber(vm, fiber)->bytecode = nost_getRef(vm, bytecode);
    int ip = 0; 

#define READ() (nost_refAsBytecode(vm, bytecode)->code.vals[ip++])

    nost_val res = nost_nilVal();

    while(true) {
        int startIp = ip;
        nost_op op = READ();
        switch(op) {
            case NOST_OP_DONE: {
                res = pop(vm, fiber); 
                goto done; 
            }
            case NOST_OP_POP: {
                pop(vm, fiber);
                break;
            }
            case NOST_OP_LOAD8: {
                uint8_t idx = READ();
                nost_val val = nost_refAsBytecode(vm, bytecode)->consts.vals[idx];
                push(vm, fiber, val); 
                break;
            }
            case NOST_OP_GET_DYNVAR: {
                nost_val name = pop(vm, fiber);
                bool valid;
                nost_val val = nost_getVar(&valid, nost_refAsFiber(vm, fiber)->ctx, name);
                if(valid) {
                    push(vm, fiber, val);
                } else {
                    nost_ref nameRef = nost_pushBlessing(vm, name);

                    error(vm, fiber, startIp, bytecode);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Variable '%s' does not exist.", nost_refAsSym(vm, nameRef)->sym);

                    nost_popBlessing(vm);
                    goto done;
                }
                break;
            }
            case NOST_OP_MAKE_DYNVAR: {
                nost_val name = pop(vm, fiber); 
                if(varInCtx(name, nost_refAsFiber(vm, fiber)->ctx)) {
                    nost_ref nameRef = nost_pushBlessing(vm, name);
                    error(vm, fiber, startIp, bytecode);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Redeclaration of variable '%s'.", nost_refAsSym(vm, nameRef)->sym);
                    nost_popBlessing(vm);
                    goto done;
                }
                nost_val val = peek(vm, fiber);
                nost_ref ctxRef = nost_pushBlessing(vm, nost_refAsFiber(vm, fiber)->ctx);
                nost_addDynvar(vm, ctxRef, name, val); 
                nost_popBlessing(vm);
                break;
            }
            case NOST_OP_PUSH_CTX: {
                nost_val newCtx = nost_makeCtx(vm, nost_refAsFiber(vm, fiber)->ctx);
                nost_writeBarrier(vm, nost_getRef(vm, fiber), newCtx);
                nost_refAsFiber(vm, fiber)->ctx = newCtx;
                break;
            }
            case NOST_OP_POP_CTX: {
                nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_asCtx(nost_refAsFiber(vm, fiber)->ctx)->parent);
                nost_refAsFiber(vm, fiber)->ctx = nost_asCtx(nost_refAsFiber(vm, fiber)->ctx)->parent;
                break;
            }
            case NOST_OP_CALL: {
                nost_val fn = pop(vm, fiber);
                if(!nost_isNatFn(fn)) {
                    error(vm, fiber, startIp, bytecode);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Cannot call %s.", nost_typename(fn));

                    goto done;
                }

                int args = READ();

                // TODO: this currently only implements native functions.

                push(vm, fiber, fn);
                nost_val* vals = NOST_GC_ALLOC(vm, args * sizeof(nost_val));
                fn = pop(vm, fiber);

                for(int i = args - 1; i >= 0; i--) {
                    vals[i] = pop(vm, fiber);
                }

                nost_natFn* natFn = nost_asNatFn(fn);
                push(vm, fiber, natFn->fn(vm, args, vals));
                NOST_GC_FREE(vm, vals, args * sizeof(nost_val));

                break;
            }
        }
    }

    done:

    nost_popBlessing(vm); // bytecode
    nost_popBlessing(vm); // fiber

    return res; 

#undef READ

}
