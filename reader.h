
#ifndef NOST_READER_H
#define NOST_READER_H

#include "val.h"
#include "vm.h"
#include "error.h"

typedef struct {
    nost_ref src;
    bool eof;
    int curr;
} nost_reader;

void nost_initReader(nost_reader* reader, nost_ref src);
nost_val nost_read(nost_vm* vm, nost_reader* reader, nost_errors* errors);
void nost_freeReader(nost_vm* vm, nost_reader* reader);

#endif
