
#ifndef NOST_PKG_H
#define NOST_PKG_H

#include "fiber.h"
#include "vm.h"
#include "sym.h"

typedef struct nost_pkg {
    nost_obj obj;
    char* name;
    nost_ctx* ctx;
} nost_pkg;

nost_pkg* nost_makePkg(nost_vm* vm, const char* name);

#endif
