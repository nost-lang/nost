
#ifndef NOST_EMBED_H
#define NOST_EMBED_H

#include "vm.h"
#include "fn.h"

void nost_addVar(nost_vm* vm, nost_ref ctx, const char* name, nost_val val);
void nost_addNatFn(nost_vm* vm, nost_ref ctx, const char* name, nost_fnPtr fn);

#endif
