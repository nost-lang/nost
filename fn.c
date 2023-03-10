
#include "fn.h"


nost_fn* nost_makeFn(nost_vm* vm, nost_sym* argName, nost_val body, nost_ctx* closureCtx, nost_pkg* pkg) {
    nost_fn* fn = (nost_fn*)nost_allocObj(vm, NOST_OBJ_FN, sizeof(nost_fn));
    fn->argName = argName;
    fn->body = body;
    fn->closureCtx = closureCtx;
    fn->macro = false;
    fn->pkg = pkg;
    return fn;
}

nost_fn* nost_makeMacro(nost_vm* vm, nost_sym* argName, nost_val body, nost_ctx* closureCtx, nost_pkg* pkg) {
    nost_fn* macro = nost_makeFn(vm, argName, body, closureCtx, pkg);
    macro->macro = true;
    return macro;
}

nost_natfn* nost_makeNatfn(nost_vm* vm, nost_val (*fn)(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args)) {
    nost_natfn* natfn = (nost_natfn*)nost_allocObj(vm, NOST_OBJ_NATFN, sizeof(nost_natfn));
    natfn->fn = fn;
    return natfn;
}
