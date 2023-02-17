
#include "gc.h"
#include "list.h"
#include "fiber.h"

static void markObj(nost_vm* vm, nost_obj* obj) {
    nost_pushDynarr(vm, &vm->grayObjs, obj);
}

static void markValue(nost_vm* vm, nost_val val) {
    if(!nost_isObj(val))
        return;
    markObj(vm, nost_asObj(val));
}

static void markRoots(nost_vm* vm) {
    for(int i = 0; i < vm->blessed.cnt; i++)
        markObj(vm, vm->blessed.vals[i]);
}

static void processGrayObj(nost_vm* vm) {
    nost_obj* obj = vm->grayObjs.vals[vm->grayObjs.cnt - 1];
    obj->marked = true;
    nost_popDynarr(&vm->grayObjs);
    switch(obj->type) {
        case NOST_OBJ_SYM:
            break;
        case NOST_OBJ_CONS: {
            nost_cons* cons = (nost_cons*)obj;
            markValue(vm, nost_car(vm, cons));
            markValue(vm, nost_cdr(vm, cons));
            break;
        }
        case NOST_OBJ_SRC:
            break;
        case NOST_OBJ_FIBER: {
            nost_fiber* fiber = (nost_fiber*)obj;
            for(int i = 0; i < fiber->ctx.dynvars.cnt; i++) {
                nost_dynvar* var = &fiber->ctx.dynvars.vals[i];
                markObj(vm, (nost_obj*)var->name);
                markValue(vm, var->val);
            }
            break;
        }
    }
}

void nost_gc(nost_vm* vm) {
    if(vm->gcPaused)
        return;
    vm->gcPaused = true;
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
            free(curr);
        } else {
            prev = curr;
        }
        curr = next;
    }
    vm->gcPaused = false;
}

void nost_bless(nost_vm* vm, nost_obj* obj) {
    vm->gcPaused = true;
    nost_pushDynarr(vm, &vm->blessed, obj);
    vm->gcPaused = false;
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
