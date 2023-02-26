
#include "pkg.h"
#include "gc.h"

nost_pkg* nost_makePkg(nost_vm* vm, const char* name) {
    nost_pkg* pkg = (nost_pkg*)nost_allocObj(vm, NOST_OBJ_PKG, sizeof(nost_pkg)); 
    nost_gcPause(vm);
    nost_pushDynarr(vm, &vm->pkgs, pkg);
    size_t nameLen = strlen(name);
    pkg->name = NOST_ALLOC(vm, nameLen + 1, "pkg name");
    memcpy(pkg->name, name, nameLen);
    pkg->name[nameLen] = '\0';
    pkg->ctx = nost_makeCtx(vm, vm->rootCtx);
    nost_gcUnpause(vm);
    return pkg;
}
