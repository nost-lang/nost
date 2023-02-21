
#include "stdlib.h"
#include "embed.h"
#include "list.h"

static nost_val cons(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args) {
    if(argc != 2) {
        nost_error err;
        nost_initError(&err, "Cons takes 2 args.");
        nost_rtError(fiber, err);
        return nost_nil();
    }
    return nost_objVal((nost_obj*)nost_makeCons(vm, args[0], args[1]));
} 

void nost_initStdlib(nost_vm* vm) {
    nost_addNatfn(vm, vm->rootCtx, "cons", cons);
}
