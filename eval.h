
#ifndef NOST_EVAL_H
#define NOST_EVAL_H

#include "value.h"
#include "vm.h"

nost_val nost_eval(nost_vm* vm, nost_ctx* ctx, nost_val expr);

#endif
