
#ifndef NOST_EMBED_H
#define NOST_EMBED_H

#include "fiber.h"

void nost_addNatfn(nost_vm* vm, nost_ctx* ctx, const char* name, nost_val (*fn)(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args));

#endif
