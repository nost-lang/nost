
#ifndef NOST_SYM_H
#define NOST_SYM_H

#include "val.h"
#include "vm.h"

typedef struct nost_sym {
    nost_obj obj; 
    int len;
    char sym[];
} nost_sym;

nost_val nost_makeSymWithLen(nost_vm* vm, int len);
void nost_initSym(nost_val sym, const char* start);

bool nost_symEq(nost_val a, nost_val b);

#endif
