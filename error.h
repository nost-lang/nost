
#ifndef NOST_ERROR_H
#define NOST_ERROR_H

#include "dynarr.h"
#include "vm.h"
#include "str.h"

typedef struct {
    enum {
        NOST_PIECE_MSG,
        NOST_PIECE_SRC_REF,
        NOST_PIECE_VAL_REF
    } type;
    union {
        char* msg;
        struct {
            int offset;
            int len;
            int lineNum;
            bool multiline;
            char* line;
        } srcRef;
        char* valRef;
    } as;
} nost_errorPiece;

typedef struct {
    nost_dynarr(nost_errorPiece) pieces;
} nost_error;

void nost_initError(nost_error* error);
void nost_freeError(nost_vm* vm, nost_error* error);
void nost_addMsg(nost_vm* vm, nost_error* error, const char* fmt, ...); 
void nost_addSrcRef(nost_vm* vm, nost_error* error, nost_ref src, int start, int end); 
void nost_addValRef(nost_vm* vm, nost_error* error, nost_val val); 
void nost_writeError(nost_vm* vm, nost_str* str, nost_error* error);

typedef nost_dynarr(nost_error) nost_errors; 

#endif
