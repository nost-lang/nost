
#include "obj.h"
#include "vm.h"
#include "list.h"
#include "fn.h"
#include "fiber.h"
#include "src.h"

nost_obj* nost_allocObj(nost_vm* vm, nost_objType type, size_t size) {
    nost_obj* obj = (nost_obj*)NOST_ALLOC(vm, size, nost_objTypenames[type]);
    obj->type = type;
    obj->next = vm->objs;
    vm->objs = obj; 
    vm->objsAllocated++;
    return obj;
}

void nost_freeObj(nost_vm* vm, nost_obj* obj) {
    switch(obj->type) {
        case NOST_OBJ_SYM: {
            nost_sym* sym = (nost_sym*)obj;
            NOST_FREE(vm, sym->sym, sym->len + 1);
            break;
        }
        case NOST_OBJ_CONS:
        case NOST_OBJ_FN:
            break;
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            nost_freeDynarr(vm, &fiber->frames);
            break;
        }

        case NOST_OBJ_NATFN:
            break;
        case NOST_OBJ_SRC: {
            nost_src* src = (nost_src*)obj;
            size_t size = strlen(src->src) + 1;
            NOST_FREE(vm, src->src, size);
            break;
        }
        case NOST_OBJ_CTX: {
            nost_ctx* ctx = (nost_ctx*)obj;
            nost_freeDynarr(vm, &ctx->dynvars);
            break;
        }
    }
    NOST_FREE(vm, obj, nost_objSize[obj->type]);
    vm->objsAllocated--;
}

size_t nost_objSize[] = {
    [NOST_OBJ_SYM] = sizeof(nost_sym),
    [NOST_OBJ_CONS] = sizeof(nost_cons),
    [NOST_OBJ_FN] = sizeof(nost_fn),
    [NOST_OBJ_FIBER] = sizeof(nost_fiber),
    [NOST_OBJ_NATFN] = sizeof(nost_natfn),
    [NOST_OBJ_SRC] = sizeof(nost_src),
    [NOST_OBJ_CTX] = sizeof(nost_ctx)
};

const char* nost_objTypenames[] = {
    [NOST_OBJ_SYM] = "sym",
    [NOST_OBJ_CONS] = "cons",
    [NOST_OBJ_FN] = "fn",
    [NOST_OBJ_FIBER] = "fiber",
    [NOST_OBJ_NATFN] = "natfn",
    [NOST_OBJ_SRC] = "src",
    [NOST_OBJ_CTX] = "ctx" 
};
