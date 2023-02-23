
#ifndef NOST_OBJ_H
#define NOST_OBJ_H

#include "common.h"

typedef enum {
    NOST_OBJ_SYM,
    NOST_OBJ_CONS,
    NOST_OBJ_FN,
    NOST_OBJ_FIBER,

    NOST_OBJ_NATFN,
    NOST_OBJ_SRC,
    NOST_OBJ_CTX
} nost_objType;

typedef struct nost_obj {
    nost_objType type;
    bool marked;
    struct nost_obj* next; 
} nost_obj;

struct nost_vm;
void nost_freeObj(struct nost_vm* vm, nost_obj* obj);
nost_obj* nost_allocObj(struct nost_vm* vm, nost_objType type, size_t size);

extern size_t nost_objSize[];
extern const char* nost_objTypenames[];

#endif
