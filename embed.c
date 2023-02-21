
#include "embed.h"
#include "gc.h"
#include "fn.h"

void nost_addNatfn(nost_vm* vm, nost_ctx* ctx, const char* name, nost_val (*fn)(nost_vm* vm, nost_fiber* fiber, int argc, nost_val* args)) {
    nost_gcPause(vm);
    nost_val fnNameVal = nost_makeSym(vm, name, strlen(name));
    if(!nost_isSym(fnNameVal))
        return;
    nost_sym* fnName = nost_asSym(fnNameVal);
    if(nost_addDynvarInCtx(vm, ctx, fnName)) {
        nost_val fnVal = nost_objVal((nost_obj*)nost_makeNatfn(vm, fn));
        nost_setVarInCtx(ctx, fnName, fnVal);
    }
    nost_gcUnpause(vm);
}