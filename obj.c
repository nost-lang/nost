
#include "obj.h"
#include "vm.h"

nost_obj* nost_allocObj(nost_vm* vm, nost_objType type, size_t size) {
    nost_obj* obj = (nost_obj*)nost_alloc(vm, size);
    obj->type = type;
    obj->next = vm->objs;
    vm->objs = obj; 
    return obj;
}
