
#ifndef NOST_VAL_H
#define NOST_VAL_H

#include "common.h"

typedef enum {
    NOST_OBJ_SYM,
    NOST_OBJ_CONS,
    NOST_OBJ_FN,
    NOST_OBJ_CLOSURE,
    NOST_OBJ_NAT_FN,
    NOST_OBJ_PKG,
    NOST_OBJ_FIBER,

    NOST_OBJ_SRC,
    NOST_OBJ_SRC_OBJ,
    NOST_OBJ_AST,
    NOST_OBJ_BYTECODE,
    NOST_OBJ_CTX,
    NOST_OBJ_PTR_FWD
} nost_objType;

#define NOST_OBJ_X \
    NOST_X_INSTANCE(sym, Sym, SYM) \
    NOST_X_INSTANCE(cons, Cons, CONS) \
    NOST_X_INSTANCE(fn, Fn, FN) \
    NOST_X_INSTANCE(closure, Closure, CLOSURE) \
    NOST_X_INSTANCE(natFn, NatFn, NAT_FN) \
    NOST_X_INSTANCE(pkg, Pkg, PKG) \
    NOST_X_INSTANCE(fiber, Fiber, FIBER) \
    NOST_X_INSTANCE(src, Src, SRC) \
    NOST_X_INSTANCE(srcObj, SrcObj, SRC_OBJ) \
    NOST_X_INSTANCE(ast, Ast, AST) \
    NOST_X_INSTANCE(bytecode, Bytecode, BYTECODE) \
    NOST_X_INSTANCE(ctx, Ctx, CTX) 

typedef struct nost_obj {
    nost_objType type;
    bool onArena;
    bool marked; // TODO: replace this with a counter for perf
    struct nost_obj* next;
} nost_obj;

typedef enum {
    NOST_VAL_NONE, // NOTE: only for use by implementation to mark absense of value(ex reader reached eof). not visible to user.
    NOST_VAL_NIL,
    NOST_VAL_NUM,
    NOST_VAL_OBJ
} nost_valType;

typedef struct {
    nost_valType type;
    union {
        double num;
        nost_obj* obj;
    } as;
} nost_val;

typedef struct nost_cons {
    nost_obj obj;
    nost_val car;
    nost_val cdr;
} nost_cons;

struct nost_vm;

nost_obj* nost_allocObj(struct nost_vm* vm, nost_objType type, size_t size);
void nost_freeObj(struct nost_vm* vm, nost_obj* obj);
size_t nost_getObjSize(nost_obj* obj);

nost_val nost_makeCons(struct nost_vm* vm);
void nost_initCons(nost_val cons, nost_val car, nost_val cdr);

nost_val nost_noneVal();
nost_val nost_nilVal();
nost_val nost_numVal(double num);
nost_val nost_objVal(nost_obj* obj);

bool nost_valOnArena(nost_val val);

bool nost_isNone(nost_val val);
bool nost_isNil(nost_val val);
bool nost_isNum(nost_val val);
bool nost_isObj(nost_val val);

double nost_asNum(nost_val val);
nost_obj* nost_asObj(nost_val val);

#ifndef NOST_BLESS_TRACK

typedef uint32_t nost_ref;

#else

typedef struct {
    uint32_t idx;
    const char* loc;
} nost_ref;

#endif

nost_val nost_getRef(struct nost_vm* vm, nost_ref ref);
void nost_setRef(struct nost_vm* vm, nost_ref ref, nost_val val);

bool nost_refIsNone(struct nost_vm* vm, nost_ref ref);
bool nost_refIsNil(struct nost_vm* vm, nost_ref ref);
bool nost_refIsNum(struct nost_vm* vm, nost_ref ref);
bool nost_refIsObj(struct nost_vm* vm, nost_ref ref);

double nost_refAsNum(struct nost_vm* vm, nost_ref ref);
nost_obj* nost_refAsObj(struct nost_vm* vm, nost_ref ref);

#define NOST_X_INSTANCE(typename, fnName, enumName) \
    bool nost_is ## fnName(nost_val val); \
    struct nost_ ## typename* nost_as ## fnName(nost_val val); \
    bool nost_refIs ## fnName(struct nost_vm* vm, nost_ref ref); \
    struct nost_ ## typename* nost_refAs ## fnName(struct nost_vm* vm, nost_ref ref);
NOST_OBJ_X
#undef NOST_X_INSTANCE

const char* nost_typename(nost_val val);
bool nost_eq(nost_val a, nost_val b);

#endif
