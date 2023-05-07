
#ifndef NOST_DEBUG_H
#define NOST_DEBUG_H

#include "bytecode.h"
#include "ast.h"

void nost_dumpAst(nost_ast* ast);
void nost_dumpBytecode(nost_bytecode* bytecode);

#endif
