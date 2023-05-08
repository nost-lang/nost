
#include "gc.h"
#include "bytecode.h"
#include "fiber.h"
#include "src.h"
#include "ast.h"

nost_ref nost_pushBlessing(nost_vm* vm, nost_val val) {
    nost_pushDynarr(vm, &vm->blessed, val);
    return vm->blessed.cnt - 1;
}

void nost_popBlessing(nost_vm* vm) {
#ifdef NOST_STRICT_MODE
    vm->blessed.vals[vm->blessed.cnt - 1] = nost_nilVal();
#endif
    nost_popDynarr(vm, &vm->blessed);
}

#ifndef NOST_GREY_TRACK
static void addToGrey(nost_vm* vm, nost_val* val) {
#else
static void addToGrey(nost_vm* vm, nost_val* val, const char* loc) {
#endif
    if(!nost_isObj(*val))
        return;
    nost_obj* obj = nost_asObj(*val);
    if(obj->onArena != vm->doingArenaGC)
        return;
    if(obj->type == NOST_OBJ_PTR_FWD) {
        *val = nost_objVal(obj->next); 
    } else {
        nost_pushDynarr(vm, &vm->grey, val);
#ifdef NOST_GREY_TRACK
        nost_pushDynarr(vm, &vm->greyLocs, loc);
#endif
    }
}

#ifndef NOST_GREY_TRACK
#define ADD_TO_GREY(vm, val) addToGrey(vm, val)
#else
#define ADD_TO_GREY(vm, val) addToGrey(vm, val, NOST_LOC())
#endif

static void traceGreyObj(nost_vm* vm, nost_obj* obj) {

    switch(obj->type) {
        case NOST_OBJ_SYM:
            break;
        case NOST_OBJ_CONS: {
            nost_cons* cons = (nost_cons*)obj;
            ADD_TO_GREY(vm, &cons->car);
            ADD_TO_GREY(vm, &cons->cdr);
            break;
        }
        case NOST_OBJ_NAT_FN:
            break;
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            ADD_TO_GREY(vm, &fiber->bytecode);
            for(int i = 0; i < fiber->stack.cnt; i++) {
                ADD_TO_GREY(vm, &fiber->stack.vals[i]);
            }
            ADD_TO_GREY(vm, &fiber->ctx);
            break;
        }
        case NOST_OBJ_SRC:
            break;
        case NOST_OBJ_SRC_OBJ: {
            nost_srcObj* srcObj = (nost_srcObj*)obj;
            ADD_TO_GREY(vm, &srcObj->src);
            ADD_TO_GREY(vm, &srcObj->val);
            break;
        }
        case NOST_OBJ_BYTECODE: {
            nost_bytecode* bytecode = (nost_bytecode*)obj;
            for(int i = 0; i < bytecode->consts.cnt; i++) {
                ADD_TO_GREY(vm, &bytecode->consts.vals[i]);
            }
            for(int i = 0; i < bytecode->errors.cnt; i++) {
                ADD_TO_GREY(vm, &bytecode->errors.vals[i].src);
            }
            break;
        }
        case NOST_OBJ_CTX: {
            nost_ctx* ctx = (nost_ctx*)obj;
            for(int i = 0; i < ctx->vars.cnt; i++) {
                ADD_TO_GREY(vm, &ctx->vars.vals[i].name);
                ADD_TO_GREY(vm, &ctx->vars.vals[i].val);
            }
            ADD_TO_GREY(vm, &ctx->parent);
            break;
        }
        case NOST_OBJ_AST: {
            nost_ast* ast = (nost_ast*)obj;
            ADD_TO_GREY(vm, &ast->src);
            switch(ast->type) {
                case NOST_AST_LITERAL: {
                    nost_astLiteral* literal = (nost_astLiteral*)ast;
                    ADD_TO_GREY(vm, &literal->val);
                    break;
                }
                case NOST_AST_VAR: {
                    nost_astVar* var = (nost_astVar*)ast;
                    ADD_TO_GREY(vm, &var->name);
                    break;
                }
                case NOST_AST_VAR_DECL: {
                    nost_astVarDecl* varDecl = (nost_astVarDecl*)ast;
                    ADD_TO_GREY(vm, &varDecl->name);
                    ADD_TO_GREY(vm, &varDecl->val);
                    break;
                }
                case NOST_AST_PROGN: {
                    nost_astProgn* progn = (nost_astProgn*)ast;
                    for(int i = 0; i < progn->nExprs; i++)
                        ADD_TO_GREY(vm, &progn->exprs[i]);
                    break;
                }
                case NOST_AST_SCOPE: {
                    nost_astScope* scope = (nost_astScope*)ast;
                    ADD_TO_GREY(vm, &scope->expr);
                    break;
                }
                case NOST_AST_IF: {
                    nost_astIf* ifAst = (nost_astIf*)ast;
                    ADD_TO_GREY(vm, &ifAst->cond);
                    ADD_TO_GREY(vm, &ifAst->thenExpr);
                    ADD_TO_GREY(vm, &ifAst->elseExpr);
                    break;
                }
                case NOST_AST_CALL: {
                    nost_astCall* call = (nost_astCall*)ast;
                    ADD_TO_GREY(vm, &call->func);
                    for(int i = 0; i < call->nArgs; i++) {
                        ADD_TO_GREY(vm, &call->args[i]);
                    }
                    ADD_TO_GREY(vm, &call->srcObj);
                    break;
                }
            }
            break;
        }
        case NOST_OBJ_PTR_FWD:
            break;
    } 
}

static void addRootsToGrey(nost_vm* vm) {

    for(int i = 0; i < vm->blessed.cnt; i++) {
        ADD_TO_GREY(vm, &vm->blessed.vals[i]);
    }

}

static void heapifyRes(nost_vm* vm, void** res, size_t size) {

    if(*res == NULL)
        return;

#ifndef NOST_MEM_TRACK

    void* alloc = *res;
    void* copy = nost_gcAlloc(vm, size);
    memcpy(copy, alloc, size);
    *res = copy;

#else

    uint8_t* alloc = *res;
    alloc -= sizeof(const char*);
    void* copy = nost_dbgGCAlloc(vm, size, *((const char**)alloc));
    memcpy(copy, alloc + sizeof(const char*), size);
    *res = copy;

#endif

}

static void heapifyObj(nost_vm* vm, nost_obj* obj) {
    switch(obj->type) {
        case NOST_OBJ_SYM:
            break;
        case NOST_OBJ_CONS:
            break;
        case NOST_OBJ_NAT_FN:
            break;
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            nost_heapifyGCDynarr(vm, &fiber->stack);
            break;
        }
        case NOST_OBJ_SRC: {
            nost_src* src = (nost_src*)obj;
            heapifyRes(vm, (void**)&src->src, src->len + 1);
            break;
        }
        case NOST_OBJ_SRC_OBJ:
            break;
        case NOST_OBJ_BYTECODE: {
            nost_bytecode* bytecode = (nost_bytecode*)obj;
            nost_heapifyGCDynarr(vm, &bytecode->code);
            nost_heapifyGCDynarr(vm, &bytecode->consts);
            nost_heapifyGCDynarr(vm, &bytecode->errors);
            break;
        }
        case NOST_OBJ_AST: {
            nost_ast* ast = (nost_ast*)obj;
            switch(ast->type) {
                case NOST_AST_LITERAL:
                    break;
                case NOST_AST_VAR:
                    break;
                case NOST_AST_VAR_DECL:
                    break;
                case NOST_AST_PROGN: {
                    nost_astProgn* progn = (nost_astProgn*)ast;
                    heapifyRes(vm, (void**)&progn->exprs, sizeof(nost_val) * progn->nExprs);
                    break;
                }
                case NOST_AST_SCOPE:
                    break;
                case NOST_AST_IF:
                    break;
                case NOST_AST_CALL: {
                    nost_astCall* call = (nost_astCall*)ast;
                    heapifyRes(vm, (void**)&call->args, sizeof(nost_val) * call->nArgs);
                    break;
                }
            }
            break;
        }
        case NOST_OBJ_CTX: {
            nost_ctx* ctx = (nost_ctx*)obj;
            nost_heapifyGCDynarr(vm, &ctx->vars);
            break;
        }
        case NOST_OBJ_PTR_FWD:
            break;
    }
}

void nost_gcArena(nost_vm* vm) {

    vm->pauseGC = true; 
    vm->doingArenaGC = true;
    int initGrey = vm->grey.cnt;

    addRootsToGrey(vm);
    for(int i = 0; i < vm->heapToArenaRoots.cnt; i++) {
        traceGreyObj(vm, nost_asObj(vm->heapToArenaRoots.vals[i]));
    }
    vm->heapToArenaRoots.cnt = 0;

    while(vm->grey.cnt > initGrey) {

        nost_val* valPtr = vm->grey.vals[vm->grey.cnt - 1];
        nost_popDynarr(vm, &vm->grey);
#ifdef NOST_GREY_TRACK
        const char* greyLoc = vm->greyLocs.vals[vm->greyLocs.cnt - 1];
        (void)greyLoc;
        nost_popDynarr(vm, &vm->greyLocs);
        NOST_ASSERT(vm->grey.cnt == vm->greyLocs.cnt, "Grey and grey loc out of sync.")
#endif

        nost_obj* obj = nost_asObj(*valPtr);
        if(!obj->onArena)
            continue;
        if(obj->type == NOST_OBJ_PTR_FWD) {
            *valPtr = nost_objVal(obj->next);
            continue;
        }

        size_t objSize = nost_getObjSize(obj);
        nost_obj* objCopy = NOST_GC_ALLOC(vm, objSize);
        vm->doingArenaGC = true;

        memcpy(objCopy, obj, objSize);
        objCopy->onArena = false;
        *valPtr = nost_objVal(objCopy);
        nost_addObjToHeap(vm, objCopy);
        heapifyObj(vm, objCopy);

        obj->type = NOST_OBJ_PTR_FWD;
        obj->next = objCopy;

        traceGreyObj(vm, objCopy);

    }

    nost_arenaClear(&vm->arena);

    vm->pauseGC = false;
    vm->doingArenaGC = false;

}

void nost_gc(nost_vm* vm) {

    if(vm->pauseGC)
        return;
    nost_gcArena(vm);
    vm->pauseGC = true;

    vm->doingArenaGC = false;
    int initGrey = vm->grey.cnt;
    
    addRootsToGrey(vm);

    while(vm->grey.cnt > initGrey) {

        nost_val* valPtr = vm->grey.vals[vm->grey.cnt - 1];
        nost_popDynarr(vm, &vm->grey);
#ifdef NOST_GREY_TRACK
        const char* greyLoc = vm->greyLocs.vals[vm->greyLocs.cnt - 1];
        (void)greyLoc;
        nost_popDynarr(vm, &vm->greyLocs);
        NOST_ASSERT(vm->grey.cnt == vm->greyLocs.cnt, "Grey and grey loc out of sync.")
#endif

        nost_obj* obj = nost_asObj(*valPtr); 
        obj->marked = true;
        traceGreyObj(vm, obj);

    }

    nost_obj* prev = NULL;
    for(nost_obj* curr = vm->objs; curr != NULL;) {
        nost_vm* newVm = vm;
        (void)newVm;
        NOST_ASSERT(curr->type != NOST_OBJ_PTR_FWD, "Pointer forward must not be on the heap list.");
        if(curr->marked) {
            prev = curr;
            curr = curr->next;
        } else {
            nost_obj* unmarked = curr;
            curr = curr->next;
            if(prev != NULL) {
                prev->next = curr;
            } else {
                vm->objs = curr;
            }
            nost_freeObj(vm, unmarked);
        }
    }

    vm->pauseGC = false;

}

void nost_addObjToHeap(nost_vm* vm, nost_obj* obj) {
    NOST_ASSERT(obj->type != NOST_OBJ_PTR_FWD, "Cannot add pointer forward to heap.");
    obj->next = vm->objs;
    vm->objs = obj;
}

void nost_writeBarrier(nost_vm* vm, nost_val refObj, nost_val newVal) {
    if(nost_valOnArena(newVal) && !nost_valOnArena(refObj))
        nost_pushDynarr(vm, &vm->heapToArenaRoots, refObj);
}
