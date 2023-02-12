
#ifndef NOST_SRC_H
#define NOST_SRC_H

#include "obj.h"

typedef struct {
    nost_obj obj;
    char* src;
} nost_src;

struct nost_vm;
nost_src* nost_source(struct nost_vm* vm, const char* src);

#endif
