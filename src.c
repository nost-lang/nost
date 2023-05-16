
#include "src.h"
#include "gc.h"

nost_val nost_makeSrc(nost_vm* vm) {
    nost_src* src = (nost_src*)nost_allocObj(vm, NOST_OBJ_SRC, sizeof(nost_src));
    src->src = NULL;
    src->len = 0;
    return nost_objVal((nost_obj*)src);
}

void nost_initSrc(nost_vm* vm, nost_val srcVal, const char* txt) {
    nost_ref src = NOST_PUSH_BLESSING(vm, srcVal);
    size_t txtLen = strlen(txt);
    char* txtCopy = NOST_RES_ALLOC(vm, txtLen + 1, nost_refAsObj(vm, src)->onArena);
    strcpy(txtCopy, txt);
    nost_refAsSrc(vm, src)->src = txtCopy; 
    nost_refAsSrc(vm, src)->len = txtLen;
    NOST_POP_BLESSING(vm, src);
}

nost_val nost_makeSrcObj(nost_vm* vm, nost_ref src, nost_ref val, int start, int end) {
    nost_srcObj* srcObj = (nost_srcObj*)nost_allocObj(vm, NOST_OBJ_SRC_OBJ, sizeof(nost_srcObj));
    srcObj->src = nost_getRef(vm, src);
    srcObj->val = nost_getRef(vm, val);
    srcObj->start = start;
    srcObj->end = end;
    return nost_objVal((nost_obj*)srcObj);
}

nost_val nost_unwrap(nost_val val) {
    if(nost_isObj(val) && nost_asObj(val)->type == NOST_OBJ_SRC_OBJ)
        return ((nost_srcObj*)nost_asObj(val))->val;
    return val;
}
