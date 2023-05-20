
#include "analysis.h"


static void analyze(nost_val ast, nost_val parent) {
    (void)parent;

    if(nost_isNil(ast))
        return;

    switch(nost_asAst(ast)->type) {
        case NOST_AST_LITERAL:
            break;
        case NOST_AST_VAR:
            break;
        case NOST_AST_VAR_DECL: {
            nost_astVarDecl* decl = nost_asAstVarDecl(ast);
            if(!nost_isNil(decl->val))
                nost_asAst(decl->val)->onReturnPath = nost_onReturnPath(ast);
            analyze(decl->val, ast);
            break;
        }
        case NOST_AST_PROGN: {
            nost_astProgn* progn = nost_asAstProgn(ast); 
            if(progn->nExprs > 0 && !nost_isNil(progn->exprs[progn->nExprs - 1]))
                nost_asAst(progn->exprs[progn->nExprs - 1])->onReturnPath = nost_onReturnPath(ast);
            for(int i = 0; i < progn->nExprs; i++)
                analyze(progn->exprs[i], ast);
            break;
        }
        case NOST_AST_SCOPE: {
            nost_astScope* scope = nost_asAstScope(ast);
            if(!nost_isNil(scope->expr))
                nost_asAst(scope->expr)->onReturnPath = nost_onReturnPath(ast);
            analyze(scope->expr, ast);
            break;
        }
        case NOST_AST_IF: {
            nost_astIf* ifAst = nost_asAstIf(ast);
            if(!nost_isNil(ifAst->thenExpr))
                nost_asAst(ifAst->thenExpr)->onReturnPath = nost_onReturnPath(ast);
            analyze(ifAst->thenExpr, ast);
            if(!nost_isNil(ifAst->elseExpr))
                nost_asAst(ifAst->elseExpr)->onReturnPath = nost_onReturnPath(ast);
            analyze(ifAst->elseExpr, ast);
            break;
        }
        case NOST_AST_CALL: {
            nost_astCall* call = nost_asAstCall(ast);
            analyze(call->func, ast);
            for(int i = 0; i < call->nArgs; i++)
                analyze(call->args[i], ast);
            break;
        }
        case NOST_AST_LAMBDA: {
            nost_astLambda* lambda = nost_asAstLambda(ast);
            if(!nost_isNil(lambda->body))
                nost_asAst(lambda->body)->onReturnPath = true;
            analyze(lambda->body, ast);
            break;
        }
        case NOST_AST_EVAL: {
            nost_astEval* eval = nost_asAstEval(ast);
            analyze(eval->expr, ast);
            break;
        }
    }

}

void nost_analyze(nost_val ast) {
    analyze(ast, nost_nilVal());
}

bool nost_onReturnPath(nost_val ast) {
    if(nost_isNil(ast))
        return false;
    return nost_asAst(ast)->onReturnPath;
}
