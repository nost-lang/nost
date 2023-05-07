
#ifndef NOST_COMPILER_H
#define NOST_COMPILER_H

#include "vm.h"
#include "bytecode.h"
#include "ast.h"

void nost_compile(nost_vm* vm, nost_ref ast, nost_ref bytecode, nost_errors* errors);

#endif
