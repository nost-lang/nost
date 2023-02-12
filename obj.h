
#ifndef NOST_OBJ_H
#define NOST_OBJ_H

#include "common.h"

typedef enum {
    NOST_OBJ_SYM,
    NOST_OBJ_CONS,

    NOST_OBJ_SRC
} nost_objType;

typedef struct {
    nost_objType type;
} nost_obj;

struct nost_vm;
nost_obj* nost_allocObj(struct nost_vm* vm, nost_objType type, size_t size);

#endif
