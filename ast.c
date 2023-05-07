
#include "ast.h"
#include "src.h"
#include "gc.h"
#include "sym.h"
#include "list.h"

size_t nost_astSize(nost_astType type) {
    switch(type) {
        case NOST_AST_LITERAL:
            return sizeof(nost_astLiteral);
        case NOST_AST_VAR:
            return sizeof(nost_astVar);
        case NOST_AST_VAR_DECL:
            return sizeof(nost_astVarDecl);
        case NOST_AST_PROGN:
            return sizeof(nost_astProgn);
        case NOST_AST_SCOPE:
            return sizeof(nost_astScope);
        case NOST_AST_CALL:
            return sizeof(nost_astCall);
    }
}

static nost_ast* allocAst(nost_vm* vm, nost_astType type, nost_val src) {
    nost_ref ref = nost_pushBlessing(vm, src);
    nost_ast* ast = (nost_ast*)nost_allocObj(vm, NOST_OBJ_AST, nost_astSize(type));
    ast->type = type;
    ast->src = nost_getRef(vm, ref); 
    nost_popBlessing(vm);
    return ast;
}

static nost_error* makeError(nost_vm* vm, nost_errors* errors) {
    nost_error error;
    nost_initError(&error);
    nost_addMsg(vm, &error, "Parsing Error.");
    nost_pushDynarr(vm, errors, error);
    nost_error* res = &errors->vals[errors->cnt - 1];
    return res;
}

static void addValEndRef(nost_vm* vm, nost_error* err, nost_val val) {
    if(nost_isSrcObj(val)) {
        nost_ref src = nost_pushBlessing(vm, nost_asSrcObj(val)->src);
        nost_addSrcRef(vm, err, src, nost_asSrcObj(val)->end - 1, nost_asSrcObj(val)->end);
        nost_popBlessing(vm);
    } else {
        nost_addValRef(vm, err, val);
    }
}

static bool carIsSym(nost_vm* vm, nost_ref val, const char* desiredSym) {
    if(!nost_isSym(nost_unwrap(nost_refCar(vm, val))))
        return false;
    nost_sym* sym = nost_asSym(nost_unwrap(nost_refCar(vm, val))); 
    return strcmp(sym->sym, desiredSym) == 0;
}

static nost_val parseVarDecl(nost_vm* vm, nost_ref val, nost_ref srcVal, nost_errors* errors) {
    int len = nost_listLen(nost_getRef(vm, val));
    if(len == 1) {
        nost_error* err = makeError(vm, errors);
        nost_addMsg(vm, err, "Expected variable name.");
        addValEndRef(vm, err, nost_getRef(vm, srcVal));
        return nost_nilVal();
    }
    if(len == 2) {
        nost_error* err = makeError(vm, errors);
        nost_addMsg(vm, err, "Expected initial value.");
        addValEndRef(vm, err, nost_getRef(vm, srcVal));
        return nost_nilVal();
    }
    if(len > 3) {
        nost_error* err = makeError(vm, errors);
        nost_addMsg(vm, err, "Unexpected values.");
        addValEndRef(vm, err, nost_getRef(vm, srcVal));
        return nost_nilVal();
    }

    nost_astVarDecl* varDecl = (nost_astVarDecl*)allocAst(vm, NOST_AST_VAR_DECL, nost_getRef(vm, srcVal));
    nost_ref declRef = nost_pushBlessing(vm, nost_objVal((nost_obj*)varDecl));

    nost_val name = nost_nth(vm, nost_getRef(vm, val), 1);
    nost_writeBarrier(vm, nost_getRef(vm, declRef), name);
    nost_refAsAstVarDecl(vm, declRef)->name = name;
    if(!nost_isSym(nost_unwrap(nost_refAsAstVarDecl(vm, declRef)->name))) {
        nost_error* err = makeError(vm, errors); 
        nost_addMsg(vm, err, "Variable name must be a symbol.");
        nost_addValRef(vm, err, nost_refAsAstVarDecl(vm, declRef)->name);
    }

    nost_val initVal = nost_parse(vm, nost_nth(vm, nost_getRef(vm, val), 2), errors);
    nost_writeBarrier(vm, nost_getRef(vm, declRef), initVal);
    nost_refAsAstVarDecl(vm, declRef)->val = initVal; 

    nost_val res = nost_getRef(vm, declRef); 
    nost_popBlessing(vm);
    return res; 
}

static nost_val parseProgn(nost_vm* vm, nost_ref val, nost_ref srcVal, nost_errors* errors) {
    nost_astProgn* progn = (nost_astProgn*)allocAst(vm, NOST_AST_PROGN, nost_getRef(vm, srcVal));
    progn->nExprs = 0;
    progn->exprs = NULL;
    nost_ref prognRef = nost_pushBlessing(vm, nost_objVal((nost_obj*)progn));

    int nExprs = nost_listLen(nost_getRef(vm, val)) - 1;
    nost_refAsAstProgn(vm, prognRef)->exprs = NOST_RES_ALLOC(vm, sizeof(nost_val) * nExprs, nost_refAsObj(vm, prognRef)->onArena);
    for(int i = 0; i < nExprs; i++)
        nost_refAsAstProgn(vm, prognRef)->exprs[i] = nost_nilVal(); 
    nost_refAsAstProgn(vm, prognRef)->nExprs = nExprs;

    nost_ref exprs = nost_pushBlessing(vm, nost_refCdr(vm, val));
    for(int i = 0; i < nExprs; i++) {
        nost_val expr = nost_refCar(vm, exprs);
        nost_val parsedExpr = nost_parse(vm, expr, errors);
        nost_writeBarrier(vm, nost_getRef(vm, prognRef), parsedExpr);
        nost_refAsAstProgn(vm, prognRef)->exprs[i] = parsedExpr;
        nost_setRef(vm, exprs, nost_refCdr(vm, exprs));
    }
    nost_popBlessing(vm);

    nost_val res = nost_getRef(vm, prognRef);
    nost_popBlessing(vm);
    return res;
}

static nost_val parseScope(nost_vm* vm, nost_ref val, nost_ref srcVal, nost_errors* errors) {

    int len = nost_listLen(nost_getRef(vm, val));

    if(len == 1) {
        nost_error* err = makeError(vm, errors);
        nost_addMsg(vm, err, "Expected expression.");
        addValEndRef(vm, err, nost_getRef(vm, srcVal));
        return nost_nilVal();
    }
    if(len > 2) {
        nost_error* err = makeError(vm, errors);
        nost_addMsg(vm, err, "Unexpected expressions.");
        addValEndRef(vm, err, nost_getRef(vm, srcVal));
        return nost_nilVal();
    }

    nost_astScope* scope = (nost_astScope*)allocAst(vm, NOST_AST_SCOPE, nost_getRef(vm, srcVal));
    nost_ref scopeRef = nost_pushBlessing(vm, nost_objVal((nost_obj*)scope));

    nost_val exprAst = nost_parse(vm, nost_nth(vm, nost_getRef(vm, val), 1), errors);
    nost_refAsAstScope(vm, scopeRef)->expr = exprAst;

    nost_val res = nost_getRef(vm, scopeRef);
    nost_popBlessing(vm);
    return res;
}

nost_val nost_parse(nost_vm* vm, nost_val srcValRaw, nost_errors* errors) {
    nost_ref srcVal = nost_pushBlessing(vm, srcValRaw);
    nost_ref val = nost_pushBlessing(vm, nost_unwrap(nost_getRef(vm, srcVal)));
    nost_val res = nost_nilVal();
    
    if(nost_refIsNil(vm, val) || nost_refIsNum(vm, val)) {
        nost_astLiteral* literal = (nost_astLiteral*)allocAst(vm, NOST_AST_LITERAL, nost_getRef(vm, srcVal));
        literal->val = nost_getRef(vm, val);
        res = nost_objVal((nost_obj*)literal);
        goto done;
    }
    if(nost_refIsSym(vm, val)) {
        nost_astVar* var = (nost_astVar*)allocAst(vm, NOST_AST_VAR, nost_getRef(vm, srcVal));
        var->name = nost_getRef(vm, srcVal);
        res = nost_objVal((nost_obj*)var);
        goto done;
    }
    if(nost_refIsCons(vm, val)) {

        // SAFETY-TODO: ensure that the cons pair is a nil-terminated list. if its not, this code will crash!

        if(carIsSym(vm, val, "var")) {
            res = parseVarDecl(vm, val, srcVal, errors);
            goto done;
        }
        if(carIsSym(vm, val, "progn")) {
            res = parseProgn(vm, val, srcVal, errors);
            goto done;
        }
        if(carIsSym(vm, val, "scope")) {
            res = parseScope(vm, val, srcVal, errors);
            goto done;
        }

        nost_astCall* call = (nost_astCall*)allocAst(vm, NOST_AST_CALL, nost_getRef(vm, srcVal));
        call->func = nost_nilVal();
        call->nArgs = 0;
        call->args = NULL;
        nost_ref callRef = nost_pushBlessing(vm, nost_objVal((nost_obj*)call));

        nost_val func = nost_parse(vm, nost_refCar(vm, val), errors);
        nost_writeBarrier(vm, nost_getRef(vm, callRef), func);
        nost_refAsAstCall(vm, callRef)->func = func;
        nost_writeBarrier(vm, nost_getRef(vm, callRef), nost_getRef(vm, srcVal));
        nost_refAsAstCall(vm, callRef)->srcObj = nost_getRef(vm, srcVal);

        int nArgs = nost_listLen(nost_getRef(vm, val)) - 1;
        nost_refAsAstCall(vm, callRef)->nArgs = 0;
        nost_refAsAstCall(vm, callRef)->args = NOST_RES_ALLOC(vm, sizeof(nost_val) * nArgs, nost_refAsObj(vm, callRef)->onArena);
        for(int i = 0; i < nArgs; i++)
            nost_refAsAstCall(vm, callRef)->args[i] = nost_nilVal();
        nost_refAsAstCall(vm, callRef)->nArgs = nArgs;

        nost_ref args = nost_pushBlessing(vm, nost_refCdr(vm, val));
        for(int i = 0; i < nArgs; i++) {
            nost_val arg = nost_refCar(vm, args);
            nost_val parsedArg = nost_parse(vm, arg, errors);
            nost_writeBarrier(vm, nost_getRef(vm, callRef), parsedArg);
            nost_refAsAstCall(vm, callRef)->args[i] = parsedArg;
            nost_setRef(vm, args, nost_refCdr(vm, args));
        }
        nost_popBlessing(vm);
        res = nost_getRef(vm, callRef);
        nost_popBlessing(vm);
    
        goto done;
    }

    nost_error* err = makeError(vm, errors);
    // TODO: consider the possibility of having values be literals by default. could be useful for macros.
    nost_addMsg(vm, err, "Cannot parse this.");
    nost_addValRef(vm, err, nost_getRef(vm, srcVal));

    done:
    nost_popBlessing(vm);
    nost_popBlessing(vm);
    return res; 
}


#define NOST_X_INSTANCE(name, nameCap) \
    bool nost_isAst ## name(nost_val val) { \
        return nost_isAst(val) && nost_asAst(val)->type == NOST_AST_ ## nameCap; \
    } \
    nost_ast ## name* nost_asAst ## name(nost_val val) { \
        return (nost_ast ## name*)nost_asAst(val); \
    } \
    bool nost_refIsAst ## name(nost_vm* vm, nost_ref ref) { \
        return nost_isAst ## name(nost_getRef(vm, ref)); \
    } \
    nost_ast ## name* nost_refAsAst ## name(nost_vm* vm, nost_ref ref) { \
        return nost_asAst ## name(nost_getRef(vm, ref)); \
    }
NOST_AST_X
#undef NOST_X_INSTANCE
