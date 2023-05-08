
#ifndef NOST_AST_H
#define NOST_AST_H

#include "val.h"
#include "vm.h"
#include "dynarr.h"
#include "error.h"
#include "common.h"

typedef enum {
    NOST_AST_LITERAL,
    NOST_AST_VAR,
    NOST_AST_VAR_DECL,
    NOST_AST_PROGN,
    NOST_AST_SCOPE,
    NOST_AST_IF,
    NOST_AST_CALL
} nost_astType;

typedef struct nost_ast {
    nost_obj obj;
    nost_astType type;
    nost_val src;
} nost_ast;

typedef struct {
    nost_ast ast;
    nost_val val;
} nost_astLiteral;

typedef struct {
    nost_ast ast;
    nost_val name;
} nost_astVar;

typedef struct {
    nost_ast ast;
    nost_val name;
    nost_val val;
} nost_astVarDecl;

typedef struct {
    nost_ast ast;
    int nExprs;
    nost_val* exprs;
} nost_astProgn;

typedef struct {
    nost_ast ast;
    nost_val expr;
} nost_astScope;

typedef struct {
    nost_ast ast;
    nost_val cond;
    nost_val thenExpr;
    nost_val elseExpr;
} nost_astIf;

typedef struct {
    nost_ast ast;
    nost_val func;
    int nArgs;
    nost_val* args;
    nost_val srcObj; // TODO: remove. this is redundant. but i'm too lazy to do it rn...
} nost_astCall;

size_t nost_astSize(nost_astType type);
nost_val nost_parse(nost_vm* vm, nost_val val, nost_errors* errors);

#define NOST_AST_X \
    NOST_X_INSTANCE(Literal, LITERAL) \
    NOST_X_INSTANCE(Var, VAR) \
    NOST_X_INSTANCE(VarDecl, VAR_DECL) \
    NOST_X_INSTANCE(Progn, PROGN) \
    NOST_X_INSTANCE(Scope, SCOPE) \
    NOST_X_INSTANCE(If, IF) \
    NOST_X_INSTANCE(Call, CALL)

#define NOST_X_INSTANCE(name, nameCap) \
    bool nost_isAst ## name(nost_val val); \
    nost_ast ## name* nost_asAst ## name(nost_val val); \
    bool nost_refIsAst ## name(nost_vm* vm, nost_ref ref); \
    nost_ast ## name* nost_refAsAst ## name(nost_vm* vm, nost_ref ref);
NOST_AST_X
#undef NOST_X_INSTANCE

#endif
