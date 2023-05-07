
#ifndef NOST_PKG_H
#define NOST_PKG_H

#include "val.h"
#include "reader.h"

typedef struct nost_pkg {
    nost_obj obj;
    nost_reader reader;
} nost_pkg;

#endif
