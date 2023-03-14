
#ifndef NOST_ERROR_H
#define NOST_ERROR_H

#include "value.h"
#include "dynarr.h"
#include "str.h"
#include "src.h"

typedef enum {
    NOST_PIECE_MESSAGE,
    NOST_PIECE_VAL_REF,
    NOST_PIECE_SRC_REF,
} nost_errorPieceType;

typedef struct {
    nost_srcRef begin;
    nost_srcRef end;
} nost_srcSpan;

typedef struct {
    nost_errorPieceType type;
    union {
        char* message;
        nost_val valRef; 
        nost_srcSpan srcRef;
    } as;
} nost_errorPiece;

typedef struct {
    nost_dynarr(nost_errorPiece) pieces;    
} nost_error;

struct nost_vm;
void nost_initError(struct nost_vm* vm, nost_error* error);
void nost_freeError(struct nost_vm* vm, nost_error* error);
void nost_addMessage(struct nost_vm* vm, nost_error* error, char* fmt, ...);
void nost_addValRef(struct nost_vm* vm, nost_error* error, nost_val ref);
void nost_addSrcRef(struct nost_vm* vm, nost_error* error, nost_srcRef begin, nost_srcRef end);
void nost_addSrcPointRef(struct nost_vm* vm, nost_error* error, nost_srcRef point);
void nost_addValEndRef(struct nost_vm* vm, nost_error* error, nost_val ref);
void nost_addListRef(struct nost_vm* vm, nost_error* error, nost_val list, int beginElem, int endElem);
void nost_doArgCntErrors(struct nost_vm* vm, nost_error* error, nost_val code, int args, const char** messages);

void nost_writeError(struct nost_vm* vm, nost_str* str, nost_error* error);

#endif
