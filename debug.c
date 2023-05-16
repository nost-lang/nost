
#include "debug.h"
#include "sym.h"
#include "src.h"

static void dumpAst(nost_val astVal, int indent) {

    if(nost_isNil(astVal)) {
        for(int i = 0; i < indent; i++)
            printf("\t");
        printf("NIL AST\n"); 
        return;
    }

    printf("[%c]", nost_asAst(astVal)->onReturnPath ? 'X' : ' ');
    for(int i = 0; i < indent; i++)
        printf("\t");
    
    nost_ast* ast = nost_asAst(astVal);

    switch(ast->type) {
        case NOST_AST_LITERAL: {
            // TODO: actually print the value. too lazy to do that rn.
            nost_astLiteral* literal = (nost_astLiteral*)ast;
            (void)literal;
            printf("literal");
            if(nost_isNum(literal->val))
                printf(" %g", nost_asNum(literal->val));
            printf("\n");
            break;
        }
        case NOST_AST_VAR: {
            nost_astVar* var = (nost_astVar*)ast;
            printf("var %s\n", nost_asSym(nost_unwrap(var->name))->sym);
            break;
        }
        case NOST_AST_VAR_DECL: {
            nost_astVarDecl* decl = (nost_astVarDecl*)ast;
            printf("var decl %s\n", nost_asSym(nost_unwrap(decl->name))->sym);
            dumpAst(decl->val, indent + 1);
            break;
        }
        case NOST_AST_PROGN: { 
            nost_astProgn* progn = (nost_astProgn*)ast;
            printf("progn\n");
            for(int i = 0; i < progn->nExprs; i++)
                dumpAst(progn->exprs[i], indent + 1);
            break;
        }
        case NOST_AST_SCOPE: {
            nost_astScope* scope = (nost_astScope*)ast;
            printf("scope\n");
            dumpAst(scope->expr, indent + 1);
            break;
        }
        case NOST_AST_IF: {
            nost_astIf* ifAst = (nost_astIf*)ast;
            printf("if\n");
            dumpAst(ifAst->cond, indent + 1);
            dumpAst(ifAst->thenExpr, indent + 1);
            if(!nost_isNil(ifAst->elseExpr)) {
                dumpAst(ifAst->elseExpr, indent + 1);
            }
            break;
        }
        case NOST_AST_CALL: {
            nost_astCall* call = (nost_astCall*)ast;
            printf("call %d\n", call->nArgs);
            dumpAst(call->func, indent + 1);
            for(int i = 0; i < call->nArgs; i++)
                dumpAst(call->args[i], indent + 1);
            break;
        }
        case NOST_AST_LAMBDA: {
            nost_astLambda* lambda = (nost_astLambda*)ast;
            printf("lambda");
            if(!nost_isNil(nost_unwrap(lambda->argName))) {
                printf(" %s", nost_asSym(nost_unwrap(lambda->argName))->sym); 
            }
            printf("\n");
            dumpAst(lambda->body, indent + 1);
            break;
        }
    }
}

void nost_dumpAst(nost_val ast) {
    dumpAst(ast, 0);
}

void nost_dumpBytecode(nost_bytecode* bytecode) {

#define READ() (bytecode->code.vals[i++])

    uint32_t read32Res;
#define READ32() do {read32Res = 0; read32Res |= READ() << 24; read32Res |= READ() << 16; read32Res |= READ() << 8; read32Res |= READ(); } while(0); 

    for(int i = 0; i < bytecode->code.cnt;) {
        printf("%d\t|  ", i);
        nost_op op = READ();
        switch(op) {
            case NOST_OP_DONE: {
                printf("done\n");
                break;
            }
            case NOST_OP_RETURN: {
                printf("return\n");
                break;
            }
            case NOST_OP_POP: {
                printf("pop\n");
                break;
            }
            case NOST_OP_LOAD8: {
                int idx = READ();
                printf("load8 %d\n", idx);
                break;
            }
            case NOST_OP_GET_DYNVAR: {
                printf("get dynvar\n");
                break;
            }
            case NOST_OP_MAKE_DYNVAR: {
                printf("make dynvar\n");
                break;
            }
            case NOST_OP_PUSH_CTX: {
                printf("push ctx\n");
                break;
            }
            case NOST_OP_POP_CTX: {
                printf("pop ctx\n");
                break;
            }
            case NOST_OP_JUMP: {
                READ32();
                printf("jump %d\n", read32Res);
                break;
            }
            case NOST_OP_JUMP_FALSE: {
                READ32();
                printf("jump if false %d\n", read32Res);
                break;
            }
            case NOST_OP_CALL: 
            case NOST_OP_TAILCALL: {
                int nArgs = READ();
                printf("%s %d\n", op == NOST_OP_TAILCALL ? "tailcall" : "call", nArgs);
                break;
            }
            case NOST_OP_MAKE_CLOSURE: {
                printf("make closure\n");
                break;
            }
            default: {
                printf("unknown opcode %d\n", op);
            }
        }
    }
#undef READ
#undef READ32
}