
#include "vm.h"
#include "src.h"
#include "gc.h"

nost_src* nost_source(nost_vm* vm, const char* src) {
    nost_src* srcObj = (nost_src*)nost_allocObj(vm, NOST_OBJ_SRC, sizeof(nost_src));
    nost_bless(vm, (nost_obj*)srcObj);
    size_t len = strlen(src);
    srcObj->src = NOST_ALLOC(vm, len + 1, "source");
    memcpy(srcObj->src, src, len + 1);
    return srcObj;
}
