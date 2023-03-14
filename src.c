
#include "vm.h"
#include "src.h"
#include "gc.h"

nost_src* nost_source(nost_vm* vm, const char* name, const char* src) {
    nost_src* srcObj = (nost_src*)nost_allocObj(vm, NOST_OBJ_SRC, sizeof(nost_src));
    nost_bless(vm, (nost_obj*)srcObj);

    if(name != NULL) {
        size_t nameLen = strlen(name);
        srcObj->name = NOST_ALLOC(vm, nameLen + 1, "srcname");
        memcpy(srcObj->name, name, nameLen + 1);
    } else {
        srcObj->name = NULL;
    }

    size_t len = strlen(src);
    srcObj->src = NOST_ALLOC(vm, len + 1, "source");
    memcpy(srcObj->src, src, len + 1);

    return srcObj;
}

void nost_initSrcRef(nost_srcRef* ref, nost_src* src, int idx) {
    ref->src = src;
    ref->idx = idx;
}

nost_val nost_makeSrcObj(nost_vm* vm, nost_src* src, nost_val val, int begin, int end) {
    nost_srcObj* srcObj = (nost_srcObj*)nost_allocObj(vm, NOST_OBJ_SRC_OBJ, sizeof(nost_srcObj));
    nost_initSrcRef(&srcObj->begin, src, begin);
    nost_initSrcRef(&srcObj->end, src, end);
    srcObj->val = val;
    return nost_objVal((nost_obj*)srcObj);
}
