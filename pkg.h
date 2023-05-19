
#ifndef NOST_PKG_H
#define NOST_PKG_H

#include "val.h"
#include "vm.h"
#include "error.h"
#include "reader.h"

typedef struct nost_pkg {
    nost_obj obj;
    nost_val ctx;
} nost_pkg;

nost_val nost_makePkg(nost_vm* vm);
nost_val nost_readAndEval(nost_vm* vm, nost_ref pkg, nost_ref fiber, nost_reader* reader, nost_errors* errs);

#endif
