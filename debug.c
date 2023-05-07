
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
            printf("literal\n");
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
    for(int i = 0; i < bytecode->code.cnt;) {
        nost_op op = READ();
        printf("%d\t|  ", i);
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
}