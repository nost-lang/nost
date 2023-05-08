
#include "compiler.h"
#include "common.h"
#include "gc.h"
#include "src.h"

static nost_error* makeError(nost_vm* vm, nost_errors* errors) {
    nost_error error;
    nost_initError(&error);
    nost_addMsg(vm, &error, "Compilation Error.");
    nost_pushDynarr(vm, errors, error);
    nost_error* res = &errors->vals[errors->cnt - 1];
    return res;
}

void nost_compile(nost_vm* vm, nost_ref ast, nost_ref bytecode, nost_errors* errors) {
    if(nost_refIsNil(vm, ast)) {
        return;
    }
    if(nost_refIsAstLiteral(vm, ast)) {
        nost_writeConst(vm, bytecode, nost_refAsAstLiteral(vm, ast)->val);
        if(nost_refAsBytecode(vm, bytecode)->consts.cnt >= 256) {
            nost_error* err = makeError(vm, errors);
            nost_addMsg(vm, err, "Constant limit exceeded. TODO: allow for >256 consts."); 
            return;
        }
        return;
    }
    if(nost_refIsAstVar(vm, ast)) {
        nost_writeConst(vm, bytecode, nost_unwrap(nost_refAsAstVar(vm, ast)->name));
        nost_writeByte(vm, bytecode, NOST_OP_GET_DYNVAR, nost_refAsAstVar(vm, ast)->name);
        return;
    }
    if(nost_refIsAstVarDecl(vm, ast)) {
        nost_ref initVal = nost_pushBlessing(vm, nost_refAsAstVarDecl(vm, ast)->val);
        nost_compile(vm, initVal, bytecode, errors);
        nost_popBlessing(vm);
        nost_writeConst(vm, bytecode, nost_unwrap(nost_refAsAstVarDecl(vm, ast)->name));
        nost_writeByte(vm, bytecode, NOST_OP_MAKE_DYNVAR, nost_refAsAst(vm, ast)->src);
        return;
    }
    if(nost_refIsAstProgn(vm, ast)) {
        for(int i = 0; i < nost_refAsAstProgn(vm, ast)->nExprs; i++) {
            nost_ref expr = nost_pushBlessing(vm, nost_refAsAstProgn(vm, ast)->exprs[i]);
            nost_compile(vm, expr, bytecode, errors);
            nost_popBlessing(vm);
            if(i != nost_refAsAstProgn(vm, ast)->nExprs - 1)
                nost_writeByte(vm, bytecode, NOST_OP_POP, nost_noneVal());
        }
        return;
    }
    if(nost_refIsAstScope(vm, ast)) {
        nost_writeByte(vm, bytecode, NOST_OP_PUSH_CTX, nost_noneVal());
        nost_ref exprRef = nost_pushBlessing(vm, nost_refAsAstScope(vm, ast)->expr);
        nost_compile(vm, exprRef, bytecode, errors);
        nost_popBlessing(vm);
        nost_writeByte(vm, bytecode, NOST_OP_POP_CTX, nost_noneVal());
        return;
    }
    if(nost_refIsAstIf(vm, ast)) {

        nost_ref condRef = nost_pushBlessing(vm, nost_refAsAstIf(vm, ast)->cond);
        nost_compile(vm, condRef, bytecode, errors);
        nost_popBlessing(vm);

        nost_writeByte(vm, bytecode, NOST_OP_JUMP_FALSE, nost_noneVal());
        int falseJumpAddr = nost_refAsBytecode(vm, bytecode)->code.cnt;
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());

        nost_ref thenRef = nost_pushBlessing(vm, nost_refAsAstIf(vm, ast)->thenExpr);
        nost_compile(vm, thenRef, bytecode, errors);
        nost_popBlessing(vm);

        nost_writeByte(vm, bytecode, NOST_OP_JUMP, nost_noneVal());
        int jumpAddr = nost_refAsBytecode(vm, bytecode)->code.cnt;
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());
        nost_writeByte(vm, bytecode, 0, nost_noneVal());

        int elseBeginAddr = nost_refAsBytecode(vm, bytecode)->code.cnt; 
        nost_patch32(vm, bytecode, falseJumpAddr, elseBeginAddr);

        if(nost_isNil(nost_refAsAstIf(vm, ast)->elseExpr)) {
            nost_writeConst(vm, bytecode, nost_nilVal());
        } else {
            nost_ref elseRef = nost_pushBlessing(vm, nost_refAsAstIf(vm, ast)->elseExpr);
            nost_compile(vm, elseRef, bytecode, errors);
            nost_popBlessing(vm);
        }

        int ifEndAddr = nost_refAsBytecode(vm, bytecode)->code.cnt; 
        nost_patch32(vm, bytecode, jumpAddr, ifEndAddr);

        return;
    }
    if(nost_refIsAstCall(vm, ast)) {

        if(nost_refAsAstCall(vm, ast)->nArgs >= 256) {
            nost_error* err = makeError(vm, errors);
            nost_addMsg(vm, err, "Call must have less than 256 arguments.");
            nost_addValRef(vm, err, nost_refAsAst(vm, ast)->src);
            return;
        }

        for(int i = 0; i < nost_refAsAstCall(vm, ast)->nArgs; i++) {
            nost_ref arg = nost_pushBlessing(vm, nost_refAsAstCall(vm, ast)->args[i]);
            nost_compile(vm, arg, bytecode, errors);
            nost_popBlessing(vm);
        }

        nost_ref func = nost_pushBlessing(vm, nost_refAsAstCall(vm, ast)->func);
        nost_compile(vm, func, bytecode, errors);

        nost_writeByte(vm, bytecode, NOST_OP_CALL, nost_refAsAst(vm, ast)->src);
        nost_writeByte(vm, bytecode, nost_refAsAstCall(vm, ast)->nArgs, nost_noneVal());

        nost_popBlessing(vm);

        return;
    }

    NOST_ASSERT(false, "Compiler given non-ast value.") 

}

