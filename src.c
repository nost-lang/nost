
#include "vm.h"
#include "src.h"

nost_src* nost_source(nost_vm* vm, const char* src) {
    nost_src* srcObj = (nost_src*)nost_allocObj(vm, NOST_OBJ_SRC, sizeof(nost_src));
    size_t len = strlen(src);
    srcObj->src = nost_alloc(vm, len + 1);
    memcpy(srcObj->src, src, len + 1);
    return srcObj;
}
