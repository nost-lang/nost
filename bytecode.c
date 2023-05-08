
#include "bytecode.h"
#include "vm.h"
#include "gc.h"

nost_val nost_makeBytecode(nost_vm* vm) {
    nost_bytecode* bytecode = (nost_bytecode*)nost_allocObj(vm, NOST_OBJ_BYTECODE, sizeof(nost_bytecode));
    nost_initGCDynarr(&bytecode->code);
    nost_initGCDynarr(&bytecode->consts);
    nost_initGCDynarr(&bytecode->errors);
    return nost_objVal((nost_obj*)bytecode);
}

void nost_writeByte(nost_vm* vm, nost_ref bytecode, uint8_t byte, nost_val src) {
    int ip = nost_refAsBytecode(vm, bytecode)->code.cnt;
    if(!nost_isNone(src)) {
        nost_ref srcRef = nost_pushBlessing(vm, src);

        nost_errorPoint err;
        err.ip = ip;
        err.src = nost_nilVal();

        nost_gcDynarr(nost_errorPoint) newErrors;
        nost_gcPushDynarr(vm, &nost_refAsBytecode(vm, bytecode)->errors, err, &newErrors);
        nost_moveGCDynarr(&newErrors, &nost_refAsBytecode(vm, bytecode)->errors);

        nost_writeBarrier(vm, nost_getRef(vm, bytecode), nost_getRef(vm, srcRef));
        nost_refAsBytecode(vm, bytecode)->errors.vals[nost_refAsBytecode(vm, bytecode)->errors.cnt - 1].src = nost_getRef(vm, srcRef);
    
        nost_popBlessing(vm);
    }
    nost_gcDynarr(uint8_t) newCode;
    nost_gcPushDynarr(vm, &nost_refAsBytecode(vm, bytecode)->code, byte, &newCode);
    nost_moveGCDynarr(&newCode, &nost_refAsBytecode(vm, bytecode)->code);
}

void nost_writeConst(nost_vm* vm, nost_ref bytecode, nost_val val) {
    nost_ref ref = nost_pushBlessing(vm, val);
    nost_writeByte(vm, bytecode, NOST_OP_LOAD8, nost_noneVal()); 
    nost_writeByte(vm, bytecode, nost_refAsBytecode(vm, bytecode)->consts.cnt, nost_noneVal());
    nost_gcDynarr(nost_val) newConsts;
    nost_gcPushDynarr(vm, &nost_refAsBytecode(vm, bytecode)->consts, nost_nilVal(), &newConsts);
    nost_writeBarrier(vm, nost_getRef(vm, bytecode), nost_getRef(vm, ref));
    newConsts.vals[newConsts.cnt - 1] = nost_getRef(vm, ref);
    nost_moveGCDynarr(&newConsts, &nost_refAsBytecode(vm, bytecode)->consts);
    nost_popBlessing(vm);
}

void nost_patch32(struct nost_vm* vm, nost_ref bytecode, int addr, uint32_t val) {
    uint8_t b0 = (val & 0xFF000000) >> 24;
    uint8_t b1 = (val & 0x00FF0000) >> 16;
    uint8_t b2 = (val & 0x0000FF00) >> 8;
    uint8_t b3 =  val & 0x000000FF;
    nost_refAsBytecode(vm, bytecode)->code.vals[addr + 0] = b0;
    nost_refAsBytecode(vm, bytecode)->code.vals[addr + 1] = b1;
    nost_refAsBytecode(vm, bytecode)->code.vals[addr + 2] = b2;
    nost_refAsBytecode(vm, bytecode)->code.vals[addr + 3] = b3;
}
