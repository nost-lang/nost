
#include "gc.h"
#include "list.h"
#include "fiber.h"
#include "fn.h"
#include "pkg.h"
#include "src.h"

static void markObj(nost_vm* vm, nost_obj* obj) {
    nost_pushDynarr(vm, &vm->grayObjs, obj);
}

static void markValue(nost_vm* vm, nost_val val) {
    if(!nost_isObj(val))
        return;
    markObj(vm, nost_asObj(val));
}

static void markRoots(nost_vm* vm) {
    markObj(vm, (nost_obj*)vm->rootCtx);
    for(int i = 0; i < vm->blessed.cnt; i++)
        markObj(vm, vm->blessed.vals[i]);
    for(int i = 0; i < vm->pkgs.cnt; i++)
        markObj(vm, (nost_obj*)vm->pkgs.vals[i]);
}

static void markError(nost_vm* vm, nost_error* error) {
    for(int i = 0; i < error->pieces.cnt; i++) {
        nost_errorPiece* piece = &error->pieces.vals[i];
        switch(piece->type) {
            case NOST_PIECE_MESSAGE:
                break;
            case NOST_PIECE_VAL_REF: {
                markValue(vm, piece->as.valRef);
                break;
            }
            case NOST_PIECE_SRC_REF: {
                markObj(vm, (nost_obj*)piece->as.srcRef.begin.src);
                markObj(vm, (nost_obj*)piece->as.srcRef.end.src);
                break;
            }
        }
    }
}

static void processGrayObj(nost_vm* vm) {
    nost_obj* obj = vm->grayObjs.vals[vm->grayObjs.cnt - 1];
    nost_popDynarr(&vm->grayObjs);
    if(obj->marked)
        return;

    obj->marked = true;
    switch(obj->type) {
        case NOST_OBJ_SYM:
            break;
        case NOST_OBJ_CONS: {
            nost_cons* cons = (nost_cons*)obj;
            markValue(vm, nost_car(vm, cons));
            markValue(vm, nost_cdr(vm, cons));
            break;
        }
        case NOST_OBJ_FN: {
            nost_fn* fn = (nost_fn*)obj;
            markObj(vm, (nost_obj*)fn->argName);
            markValue(vm, fn->body);
            markObj(vm, (nost_obj*)fn->closureCtx);
            if(fn->pkg != NULL)
                markObj(vm, (nost_obj*)fn->pkg);
            break;
        }
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            for(int i = 0; i < fiber->frames.cnt; i++) {
                nost_frame* frame = &fiber->frames.vals[i];
                markObj(vm, (nost_obj*)frame->currCtx);
                if(frame->pkg != NULL)
                    markObj(vm, (nost_obj*)frame->pkg);
                markValue(vm, frame->callsite);
            }
            break;
        }
        case NOST_OBJ_PKG: {
            nost_pkg* pkg = (nost_pkg*)obj;
            markObj(vm, (nost_obj*)pkg->ctx); 
            break;
        }
        case NOST_OBJ_NATFN:
            break;
        case NOST_OBJ_SRC:
            break;
        case NOST_OBJ_SRC_OBJ: {
            nost_srcObj* srcObj = (nost_srcObj*)obj;
            markObj(vm, (nost_obj*)srcObj->begin.src);
            markObj(vm, (nost_obj*)srcObj->end.src);
            markValue(vm, srcObj->val);
            break;
        }
        case NOST_OBJ_CTX: {
            nost_ctx* ctx = (nost_ctx*)obj;
            for(int i = 0; i < ctx->dynvars.cnt; i++) {
                nost_dynvar* var = &ctx->dynvars.vals[i];
                markObj(vm, (nost_obj*)var->name);
                markValue(vm, var->val);
                markValue(vm, var->decl);
            }
            if(ctx->parent != NULL)
                markObj(vm, (nost_obj*)ctx);
            break;
        }
    }
}

void nost_gc(nost_vm* vm) {
    if(vm->gcPaused > 0)
        return;
    nost_gcPause(vm);
    for(nost_obj* curr = vm->objs; curr != NULL; curr = curr->next)
        curr->marked = false;
    markRoots(vm); 
    while(vm->grayObjs.cnt > 0)
        processGrayObj(vm);
    nost_obj* prev = NULL;
    for(nost_obj* curr = vm->objs; curr != NULL;) {
        nost_obj* next = curr->next;
        if(!curr->marked) {
            if(prev == NULL) {
                vm->objs = curr->next;
            } else {
                prev->next = curr->next;
            } 
            nost_freeObj(vm, curr);
        } else {
            prev = curr;
        }
        curr = next;
    }
    nost_gcUnpause(vm);
}

void nost_bless(nost_vm* vm, nost_obj* obj) {
    nost_gcPause(vm);
    nost_pushDynarr(vm, &vm->blessed, obj);
    nost_gcUnpause(vm);
}

void nost_blessVal(nost_vm* vm, nost_val val) {
    if(nost_isObj(val))
        nost_bless(vm, nost_asObj(val));
}

void nost_unbless(nost_vm* vm, nost_obj* obj) {
    // TODO: make some kind of lookup for this stored in obj header
    // probly not a major perf problem, but hey
    for(int i = 0; i < vm->blessed.cnt; i++) {
        if(vm->blessed.vals[i] == obj) {
            vm->blessed.vals[i] = vm->blessed.vals[vm->blessed.cnt - 1];
            nost_popDynarr(&vm->blessed);
        }
    }
}

void nost_unblessVal(nost_vm* vm, nost_val val) { 
    if(nost_isObj(val))
        nost_unbless(vm, nost_asObj(val));
}

void nost_gcPause(nost_vm* vm) {
    vm->gcPaused++;
}

void nost_gcUnpause(nost_vm* vm) {
    vm->gcPaused--;
}
