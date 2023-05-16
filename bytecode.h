
#ifndef NOST_BYTECODE_H
#define NOST_BYTECODE_H

#include "val.h"
#include "dynarr.h"
#include "common.h"

typedef enum {
    NOST_OP_DONE, // stop executing bytecode. placed at end of toplevel exec 
    NOST_OP_RETURN, // exit current frame 

    NOST_OP_POP,

    NOST_OP_LOAD8, // TODO: add support for more than 256 consts

    NOST_OP_GET_DYNVAR,
    NOST_OP_MAKE_DYNVAR,
    NOST_OP_PUSH_CTX,
    NOST_OP_POP_CTX,

    NOST_OP_JUMP,
    NOST_OP_JUMP_FALSE,

    NOST_OP_CALL,
    NOST_OP_TAILCALL,

    NOST_OP_MAKE_CLOSURE
} nost_op;

typedef struct {
    int ip;
    nost_val src;
} nost_errorPoint;

typedef struct nost_bytecode {
    nost_obj obj;
    nost_gcDynarr(uint8_t) code; 
    nost_gcDynarr(nost_val) consts;
    nost_gcDynarr(nost_errorPoint) errors;
} nost_bytecode;

struct nost_vm;

nost_val nost_makeBytecode(struct nost_vm* vm);
void nost_writeByte(struct nost_vm* vm, nost_ref bytecode, uint8_t byte, nost_val src);
void nost_writeConst(struct nost_vm* vm, nost_ref bytecode, nost_val src);
void nost_patch32(struct nost_vm* vm, nost_ref bytecode, int addr, uint32_t val);

#endif
