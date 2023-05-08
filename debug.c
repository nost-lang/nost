
#include "debug.h"
#include "sym.h"
#include "src.h"

static void dumpAst(nost_ast* ast, int indent) {

    // TODO: this is bad. nil val not guaranteed to have a null pointer. too bad! 
    if(ast == NULL)
        return;

    for(int i = 0; i < indent; i++)
        printf("\t");
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
            dumpAst(nost_asAst(decl->val), indent + 1);
            break;
        }
        case NOST_AST_PROGN: { 
            nost_astProgn* progn = (nost_astProgn*)ast;
            printf("progn\n");
            for(int i = 0; i < progn->nExprs; i++)
                dumpAst(nost_asAst(progn->exprs[i]), indent + 1);
            break;
        }
        case NOST_AST_SCOPE: {
            nost_astScope* scope = (nost_astScope*)ast;
            printf("scope\n");
            dumpAst(nost_asAst(scope->expr), indent + 1);
            break;
        }
        case NOST_AST_IF: {
            nost_astIf* ifAst = (nost_astIf*)ast;
            printf("if\n");
            dumpAst(nost_asAst(ifAst->cond), indent + 1);
            dumpAst(nost_asAst(ifAst->thenExpr), indent + 1);
            if(!nost_isNil(ifAst->elseExpr)) {
                dumpAst(nost_asAst(ifAst->elseExpr), indent + 1);
            }
            break;
        }
        case NOST_AST_CALL: {
            nost_astCall* call = (nost_astCall*)ast;
            printf("call %d\n", call->nArgs);
            dumpAst(nost_asAst(call->func), indent + 1);
            for(int i = 0; i < call->nArgs; i++)
                dumpAst(nost_asAst(call->args[i]), indent + 1);
            break;
        }
    }
}

void nost_dumpAst(nost_ast* ast) {
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
            case NOST_OP_CALL: {
                int nArgs = READ();
                printf("call %d\n", nArgs);
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