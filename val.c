
#include "val.h"
#include "vm.h"
#include "mem.h"
#include "gc.h"
#include "bytecode.h"
#include "fiber.h"
#include "src.h"
#include "ast.h"
#include "sym.h"
#include "fn.h"

nost_obj* nost_allocObj(nost_vm* vm, nost_objType type, size_t size) {
    nost_obj* obj = nost_arenaAlloc(&vm->arena, size);
    if(obj == NULL) {
        nost_gcArena(vm);
        obj = nost_arenaAlloc(&vm->arena, size);
        if(obj == NULL) {
            obj = NOST_GC_ALLOC(vm, size);
            obj->onArena = false;
            nost_addObjToHeap(vm, obj);
        } else {
            obj->onArena = true;
        }
    } else {
        obj->onArena = true;
    }
    obj->type = type;
    return obj;
}

void nost_freeObj(nost_vm* vm, nost_obj* obj) {
    switch(obj->type) {
        case NOST_OBJ_SYM:
            break;
        case NOST_OBJ_CONS:
            break;
        case NOST_OBJ_NAT_FN:
            break;
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            nost_freeGCDynarr(vm, &fiber->stack);
            break;
        }
        case NOST_OBJ_SRC: {
            nost_src* src = (nost_src*)obj;
            NOST_RES_FREE(vm, src->src, src->len + 1);
            break;
        }
        case NOST_OBJ_SRC_OBJ:
            break;
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
                    NOST_RES_FREE(vm, progn->exprs, sizeof(nost_val) * progn->nExprs);
                    break;
                }
                case NOST_AST_SCOPE:
                    break;
                case NOST_AST_IF:
                    break;
                case NOST_AST_CALL: {
                    nost_astCall* call = (nost_astCall*)ast;
                    NOST_RES_FREE(vm, call->args, sizeof(nost_val) * call->nArgs);
                    break;
                }
            } 
            break;
        }
        case NOST_OBJ_BYTECODE: {
            nost_bytecode* bytecode = (nost_bytecode*)obj;
            nost_freeGCDynarr(vm, &bytecode->code); 
            nost_freeGCDynarr(vm, &bytecode->consts);
            nost_freeGCDynarr(vm, &bytecode->errors);
            break;
        }
        case NOST_OBJ_CTX: {
            nost_ctx* ctx = (nost_ctx*)obj;
            nost_freeGCDynarr(vm, &ctx->vars);
            break;
        }
        case NOST_OBJ_PTR_FWD:
            break;
    }
    size_t objSize = nost_getObjSize(obj);
    NOST_GC_FREE(vm, obj, objSize);
}

size_t nost_getObjSize(nost_obj* obj) {
    switch(obj->type) {
        case NOST_OBJ_SYM: {
            nost_sym* sym = (nost_sym*)obj;
            return sizeof(nost_sym) + sym->len;
        }
        case NOST_OBJ_CONS:
            return sizeof(nost_cons);
        case NOST_OBJ_NAT_FN:
            return sizeof(nost_natFn);
        case NOST_OBJ_FIBER:
            return sizeof(nost_fiber);
        case NOST_OBJ_SRC:
            return sizeof(nost_src);
        case NOST_OBJ_SRC_OBJ:
            return sizeof(nost_srcObj);
        case NOST_OBJ_BYTECODE:
            return sizeof(nost_bytecode);
        case NOST_OBJ_AST: {
            nost_ast* ast = (nost_ast*)obj;
            return nost_astSize(ast->type);
        }
        case NOST_OBJ_CTX:
            return sizeof(nost_ctx);
        case NOST_OBJ_PTR_FWD:
            return 0;
    }
}

nost_val nost_makeCons(struct nost_vm* vm) {
    nost_cons* cons = (nost_cons*)nost_allocObj(vm, NOST_OBJ_CONS, sizeof(nost_cons));
    cons->car = nost_nilVal();
    cons->cdr = nost_nilVal();
    return nost_objVal((nost_obj*)cons);
}

void nost_initCons(nost_val cons, nost_val car, nost_val cdr) {
    nost_asCons(cons)->car = car;
    nost_asCons(cons)->cdr = cdr;
}

nost_val nost_noneVal() {
    return (nost_val){NOST_VAL_NONE, {.num = 0}};
}

nost_val nost_nilVal() {
    return (nost_val){NOST_VAL_NIL, {.num = 0}};
}

nost_val nost_numVal(double num) {
    return (nost_val){NOST_VAL_NUM, {.num = num}};
}

nost_val nost_objVal(nost_obj* obj) {
    return (nost_val){NOST_VAL_OBJ, {.obj = obj}};
}

bool nost_valOnArena(nost_val val) {
    if(!nost_isObj(val))
        return false;
    return nost_asObj(val)->onArena;
}

bool nost_isNone(nost_val val) {
    return val.type == NOST_VAL_NONE;
}

bool nost_isNil(nost_val val) {
    return val.type == NOST_VAL_NIL;
}

bool nost_isNum(nost_val val) {
    return val.type == NOST_VAL_NUM;
}

bool nost_isObj(nost_val val) {
    return val.type == NOST_VAL_OBJ;
}

double nost_asNum(nost_val val) {
    return val.as.num;
}

nost_obj* nost_asObj(nost_val val) {
    return val.as.obj;
}


nost_val nost_getRef(nost_vm* vm, nost_ref ref) {
    NOST_ASSERT(vm->blessed.cnt > ref, "Invalid ref.");
    return vm->blessed.vals[ref];
}

void nost_setRef(nost_vm* vm, nost_ref ref, nost_val val) {
    vm->blessed.vals[ref] = val;
}

bool nost_refIsNil(nost_vm* vm, nost_ref ref) {
    return nost_isNil(nost_getRef(vm, ref));
}

bool nost_refIsNum(nost_vm* vm, nost_ref ref) {
    return nost_isNum(nost_getRef(vm, ref));
}

bool nost_refIsObj(nost_vm* vm, nost_ref ref) {
    return nost_isObj(nost_getRef(vm, ref));
}

double nost_refAsNum(nost_vm* vm, nost_ref ref) {
    return nost_asNum(nost_getRef(vm, ref));
}

nost_obj* nost_refAsObj(nost_vm* vm, nost_ref ref) {
    return nost_asObj(nost_getRef(vm, ref));
}

#define NOST_X_INSTANCE(typename, fnName, enumName) \
    bool nost_is ## fnName(nost_val val) { \
        return nost_isObj(val) && nost_asObj(val)->type == NOST_OBJ_ ## enumName; \
    } \
    struct nost_ ## typename* nost_as ## fnName(nost_val val) { \
        NOST_ASSERT(nost_asObj(val)->type == NOST_OBJ_ ## enumName, "Cannot cast this value to " #typename); \
        return (nost_ ## typename*)nost_asObj(val); \
    } \
    bool nost_refIs ## fnName(struct nost_vm* vm, nost_ref ref) { \
        return nost_is ## fnName(nost_getRef(vm, ref)); \
    } \
    struct nost_ ## typename* nost_refAs ## fnName(struct nost_vm* vm, nost_ref ref) { \
        return nost_as ## fnName(nost_getRef(vm, ref)); \
    }
NOST_OBJ_X
#undef NOST_X_INSTANCE

const char* nost_typename(nost_val val) {
    if(nost_isNil(val))
        return "nil";
    if(nost_isNum(val))
        return "num";
    if(nost_isObj(val)) {
        switch(nost_asObj(val)->type) {
            case NOST_OBJ_SYM:
                return "sym";
            case NOST_OBJ_CONS:
                return "cons"; 
            case NOST_OBJ_NAT_FN:
                return "fn";
            case NOST_OBJ_FIBER:
                return "fiber";
            case NOST_OBJ_SRC:
            case NOST_OBJ_SRC_OBJ:
            case NOST_OBJ_AST:
            case NOST_OBJ_BYTECODE:
            case NOST_OBJ_CTX:
            case NOST_OBJ_PTR_FWD: {
                NOST_ASSERT(false, "Internal type must not be user visible!");
            }
        }
    }

    NOST_ASSERT(false, "Some typename is missing.");
    return "XXX";
}

