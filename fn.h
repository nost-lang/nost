
#ifndef NOST_FN_H
#define NOST_FN_H

#include "vm.h"
#include "obj.h"
#include "value.h"
#include "fiber.h"
#include "pkg.h"

typedef struct nost_fn {
    nost_obj obj;
    nost_sym* argName;
    nost_val body;
    nost_ctx* closureCtx;
    nost_pkg* pkg;
    bool macro;
} nost_fn;

nost_fn* nost_makeFn(nost_vm* vm, nost_sym* argName, nost_val body, nost_ctx* closureCtx, nost_pkg* pkg);
nost_fn* nost_makeMacro(nost_vm* vm, nost_sym* argName, nost_val body, nost_ctx* closureCtx, nost_pkg* pkg);

typedef struct nost_natfn {
    nost_obj obj;
    nost_val (*fn)(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args); 
} nost_natfn;

nost_natfn* nost_makeNatfn(nost_vm* vm, nost_val (*fn)(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args));

#endif
