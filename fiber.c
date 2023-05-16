
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
    nost_ref name = NOST_PUSH_BLESSING(vm, nameVal);
    nost_ref val = NOST_PUSH_BLESSING(vm, valVal);

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

    NOST_POP_BLESSING(vm, val);
    NOST_POP_BLESSING(vm, name);
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

nost_val nost_makeFiber(nost_vm* vm) {
    nost_fiber* fiber = (nost_fiber*)nost_allocObj(vm, NOST_OBJ_FIBER, sizeof(nost_fiber));
    fiber->hadError = false;
    nost_initGCDynarr(&fiber->stack);
    nost_initGCDynarr(&fiber->frames);
    nost_ref ref = NOST_PUSH_BLESSING(vm, nost_objVal((nost_obj*)fiber));

    nost_val res = nost_objVal(nost_refAsObj(vm, ref));
    NOST_POP_BLESSING(vm, ref);
    return res;
}

static void push(nost_vm* vm, nost_ref fiber, nost_val val) {
    // TODO: this amount of nonsense for a core vm operation is horrible.
    // potential solution: make every existing fiber's stack an implicit GC root 
    nost_ref ref = NOST_PUSH_BLESSING(vm, val);
    nost_gcDynarr(nost_val) oldStack;
    nost_moveGCDynarr(&nost_refAsFiber(vm, fiber)->stack, &oldStack);
    nost_gcDynarr(nost_val) newStack;
    nost_gcPushDynarr(vm, &oldStack, nost_nilVal(), &newStack);
    nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_getRef(vm, ref));
    newStack.vals[newStack.cnt - 1] = nost_getRef(vm, ref);
    nost_moveGCDynarr(&newStack, &nost_refAsFiber(vm, fiber)->stack);
    NOST_POP_BLESSING(vm, ref);
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

static nost_frame* currFrame(nost_vm* vm, nost_ref fiber) {
    nost_fiber* fiberPtr = nost_refAsFiber(vm, fiber);
    return &fiberPtr->frames.vals[fiberPtr->frames.cnt - 1];
} 

static void pushFrame(nost_vm* vm, nost_ref fiber, nost_val bytecode, nost_val ctx) {
    nost_ref bytecodeRef = NOST_PUSH_BLESSING(vm, bytecode);
    nost_ref ctxRef = NOST_PUSH_BLESSING(vm, ctx);

    nost_frame frame;
    frame.bytecode = nost_nilVal();
    frame.ctx = nost_nilVal();
    frame.ip = 0;
    nost_gcDynarr(nost_frame) newFrames;
    nost_gcPushDynarr(vm, &nost_refAsFiber(vm, fiber)->frames, frame, &newFrames);
    nost_moveGCDynarr(&newFrames, &nost_refAsFiber(vm, fiber)->frames);

    nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_getRef(vm, bytecodeRef));
    currFrame(vm, fiber)->bytecode = nost_getRef(vm, bytecodeRef);
    nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_getRef(vm, ctxRef));
    currFrame(vm, fiber)->ctx = nost_getRef(vm, ctxRef);

    NOST_POP_BLESSING(vm, ctxRef);
    NOST_POP_BLESSING(vm, bytecodeRef);
}

static void popFrame(nost_vm* vm, nost_ref fiber) {
    nost_popDynarr(vm, &nost_refAsFiber(vm, fiber)->frames);
}

static nost_bytecode* currBytecode(nost_vm* vm, nost_ref fiber) {
    return nost_asBytecode(currFrame(vm, fiber)->bytecode);
}

static nost_val currCtx(nost_vm* vm, nost_ref fiber) {
    return currFrame(vm, fiber)->ctx;
}


static void error(nost_vm* vm, nost_ref fiber, int ip) {
    nost_refAsFiber(vm, fiber)->hadError = true;
    nost_initError(&nost_refAsFiber(vm, fiber)->err);
    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Runtime Error.");
    // TODO: add full stack trace here
    bool found = false;
    for(int i = 0; i < currBytecode(vm, fiber)->errors.cnt; i++) {
        if(currBytecode(vm, fiber)->errors.vals[i].ip == ip) {
            nost_addValRef(vm, &nost_refAsFiber(vm, fiber)->err, currBytecode(vm, fiber)->errors.vals[i].src);
            found = true;
            break;
        }
    }
    NOST_ASSERT(found, "Specified IP not found in bytecode error points.");
}

nost_val nost_execBytecode(nost_vm* vm, nost_ref fiber, nost_val bytecodeVal, nost_val ctx) {

    nost_ref bytecodeRef = NOST_PUSH_BLESSING(vm, bytecodeVal);  
    pushFrame(vm, fiber, nost_getRef(vm, bytecodeRef), ctx);
    NOST_POP_BLESSING(vm, bytecodeRef);

#define READ() (currBytecode(vm, fiber)->code.vals[currFrame(vm, fiber)->ip++])

    uint32_t read32Res;
#define READ32() do {read32Res = 0; read32Res |= READ() << 24; read32Res |= READ() << 16; read32Res |= READ() << 8; read32Res |= READ(); } while(0); 

    nost_val res = nost_nilVal();

    while(true) {
        int startIp = currFrame(vm, fiber)->ip;
        nost_op op = READ();
        switch(op) {
            case NOST_OP_DONE: {
                res = pop(vm, fiber); 
                goto done; 
            }
            case NOST_OP_RETURN: {
                popFrame(vm, fiber); 
                break;
            }
            case NOST_OP_POP: {
                pop(vm, fiber);
                break;
            }
            case NOST_OP_LOAD8: {
                uint8_t idx = READ();
                nost_val val = currBytecode(vm, fiber)->consts.vals[idx];
                push(vm, fiber, val); 
                break;
            }
            case NOST_OP_GET_DYNVAR: {
                nost_val name = pop(vm, fiber);
                bool valid;
                nost_val val = nost_getVar(&valid, currCtx(vm, fiber), name);
                if(valid) {
                    push(vm, fiber, val);
                } else {
                    nost_ref nameRef = NOST_PUSH_BLESSING(vm, name);

                    error(vm, fiber, startIp);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Variable '%s' does not exist.", nost_refAsSym(vm, nameRef)->sym);

                    NOST_POP_BLESSING(vm, nameRef);
                    goto done;
                }
                break;
            }
            case NOST_OP_MAKE_DYNVAR: {
                nost_val name = pop(vm, fiber); 
                if(varInCtx(name, currCtx(vm, fiber))) {
                    nost_ref nameRef = NOST_PUSH_BLESSING(vm, name);
                    error(vm, fiber, startIp);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Redeclaration of variable '%s'.", nost_refAsSym(vm, nameRef)->sym);
                    // TODO: add an "original declaration here" part to the error
                    NOST_POP_BLESSING(vm, nameRef);
                    goto done;
                }
                nost_val val = peek(vm, fiber);
                nost_ref ctxRef = NOST_PUSH_BLESSING(vm, currCtx(vm, fiber));
                nost_addDynvar(vm, ctxRef, name, val); 
                NOST_POP_BLESSING(vm, ctxRef);
                break;
            }
            case NOST_OP_PUSH_CTX: {
                nost_val newCtx = nost_makeCtx(vm, currCtx(vm, fiber));
                nost_writeBarrier(vm, nost_getRef(vm, fiber), newCtx);
                currFrame(vm, fiber)->ctx = newCtx;
                break;
            }
            case NOST_OP_POP_CTX: {
                nost_writeBarrier(vm, nost_getRef(vm, fiber), nost_asCtx(currCtx(vm, fiber))->parent);
                currFrame(vm, fiber)->ctx = nost_asCtx(currCtx(vm, fiber))->parent;
                break;
            }
            case NOST_OP_JUMP: {
                READ32();
                currFrame(vm, fiber)->ip = read32Res;
                break;
            }
            case NOST_OP_JUMP_FALSE: {
                READ32();
                if(nost_isNil(pop(vm, fiber))) {
                    currFrame(vm, fiber)->ip = read32Res;
                }
                break;
            }
            case NOST_OP_CALL: {

                if(nost_refAsFiber(vm, fiber)->frames.cnt == 4096) {
                    error(vm, fiber, startIp);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Stack overflow."); 
                    goto done;
                }

                nost_val fn = pop(vm, fiber);
                if(!nost_isClosure(fn) && !nost_isNatFn(fn)) {
                    error(vm, fiber, startIp);
                    nost_addMsg(vm, &nost_refAsFiber(vm, fiber)->err, "Cannot call %s.", nost_typename(fn));
                    goto done;
                }

                int args = READ();

                if(nost_isClosure(fn)) {
                    nost_ref fnRef = NOST_PUSH_BLESSING(vm, fn);

                    nost_ref argList = NOST_PUSH_BLESSING(vm, nost_nilVal());
                    for(int i = 0; i < args; i++) {
                        nost_val cons = nost_makeCons(vm);
                        nost_initCons(cons, pop(vm, fiber), nost_getRef(vm, argList));
                        nost_setRef(vm, argList, cons); 
                    }
                    push(vm, fiber, nost_getRef(vm, argList));
                    NOST_POP_BLESSING(vm, argList);

                    nost_val newCtx = nost_makeCtx(vm, nost_refAsClosure(vm, fnRef)->closureCtx);
                    pushFrame(vm, fiber, nost_asFn(nost_refAsClosure(vm, fnRef)->fn)->bytecode, newCtx);

                    NOST_POP_BLESSING(vm, fnRef);
                } else if(nost_isNatFn(fn)) {
                    push(vm, fiber, fn);
                    nost_val* vals = NOST_GC_ALLOC(vm, args * sizeof(nost_val));
                    fn = pop(vm, fiber);

                    for(int i = args - 1; i >= 0; i--) {
                        vals[i] = pop(vm, fiber);
                    }

                    nost_natFn* natFn = nost_asNatFn(fn);
                    push(vm, fiber, natFn->fn(vm, args, vals));
                    NOST_GC_FREE(vm, vals, args * sizeof(nost_val));
                }

                break;
            }
            case NOST_OP_MAKE_CLOSURE: {
                nost_val fn = pop(vm, fiber);
                nost_val ctx = currCtx(vm, fiber);
                nost_val closure = nost_makeClosure(vm, fn, ctx);
                push(vm, fiber, closure);
                break;
            }
        }
    }

    done:

    return res; 

#undef READ
#undef READ32

}
