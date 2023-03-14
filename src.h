
#ifndef NOST_SRC_H
#define NOST_SRC_H

#include "obj.h"

typedef struct {
    nost_obj obj;
    char* name;
    char* src;
} nost_src;

struct nost_vm;
nost_src* nost_source(struct nost_vm* vm, const char* name, const char* src);

typedef struct {
    nost_src* src;
    int idx;
} nost_srcRef;

typedef struct nost_srcObj {
    nost_obj obj;
    nost_srcRef begin, end;
    nost_val val;
} nost_srcObj;

struct nost_vm;
void nost_initSrcRef(nost_srcRef* ref, nost_src* src, int idx);
nost_val nost_makeSrcObj(struct nost_vm* vm, nost_src* src, nost_val val, int begin, int end);

#endif
