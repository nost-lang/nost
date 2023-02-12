
#ifndef NOST_LIST_H
#define NOST_LIST_H

#include "value.h"

typedef struct nost_cons {
    nost_obj obj;
    nost_val car;
    nost_val cdr;
} nost_cons;

struct nost_vm;
nost_cons* nost_makeCons(struct nost_vm* vm, nost_val car, nost_val cdr);
nost_val nost_list(struct nost_vm* vm, int cnt, nost_val* elems);
nost_val nost_car(struct nost_vm* vm, nost_cons* cons);
nost_val nost_cdr(struct nost_vm* vm, nost_cons* cons);
bool nost_nilTerminated(struct nost_vm* vm, nost_val list);
int nost_len(struct nost_vm* vm, nost_val list);
nost_val nost_nth(struct nost_vm* vm, nost_val list, int n);

#endif
