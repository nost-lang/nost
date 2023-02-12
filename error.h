
#ifndef NOST_ERROR_H
#define NOST_ERROR_H

typedef struct {
    const char* msg;
} nost_error;

void nost_initError(nost_error* error, const char* msg);

#endif
