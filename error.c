
#include "error.h"

void nost_initError(nost_error* error, const char* msg) {
    error->msg = msg;
}
