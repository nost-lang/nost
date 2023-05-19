
#include "embed.h"
#include "gc.h"
#include "sym.h"
#include "fiber.h"

void nost_addVar(nost_vm* vm, nost_ref ctx, const char* name, nost_val val) {
    nost_ref valRef = NOST_PUSH_BLESSING(vm, val);

    nost_val nameSym = nost_makeSymWithLen(vm, strlen(name));
    nost_initSym(nameSym, name);

    // TODO: add some kind of check here to make sure variable isn't redefined
    nost_addDynvar(vm, ctx, nameSym, nost_getRef(vm, valRef));
    
    NOST_POP_BLESSING(vm, valRef);
}

void nost_addNatFn(nost_vm* vm, nost_ref ctx, const char* name, nost_fnPtr fn) {
    nost_addVar(vm, ctx, name, nost_makeNatFn(vm, fn));
}
