
#include "pkg.h"
#include "fiber.h"
#include "gc.h"
#include "ast.h"
#include "analysis.h"
#include "bytecode.h"
#include "compiler.h"

nost_val nost_makePkg(nost_vm* vm) {

    nost_val ctx = nost_makeCtx(vm, vm->stdlibCtx);
    nost_ref ctxRef = NOST_PUSH_BLESSING(vm, ctx);

    nost_pkg* pkg = (nost_pkg*)nost_allocObj(vm, NOST_OBJ_PKG, sizeof(nost_pkg));
    
    nost_writeBarrier(vm, nost_objVal((nost_obj*)pkg), ctx);
    pkg->ctx = nost_getRef(vm, ctxRef);

    NOST_POP_BLESSING(vm, ctxRef);

    return nost_objVal((nost_obj*)pkg);
}

nost_val nost_readAndEval(nost_vm* vm, nost_ref pkg, nost_ref fiber, nost_reader* reader, nost_errors* errs) {

    nost_val code = nost_read(vm, reader, errs);
    if(nost_isNone(code))
        return nost_noneVal();
    
    nost_ref ast = NOST_PUSH_BLESSING(vm, nost_parse(vm, code, errs));

    nost_analyze(nost_getRef(vm, ast)); 

    nost_ref bytecode = NOST_PUSH_BLESSING(vm, nost_makeBytecode(vm));
    nost_compile(vm, ast, bytecode, errs);
    nost_writeByte(vm, bytecode, NOST_OP_DONE, nost_nilVal());

    if(errs->cnt > 0) {
        NOST_POP_BLESSING(vm, bytecode);
        NOST_POP_BLESSING(vm, ast);
        return nost_noneVal();
    }

    nost_val res = nost_execBytecode(vm, fiber, nost_getRef(vm, bytecode), nost_refAsPkg(vm, pkg)->ctx);

    if(nost_refAsFiber(vm, fiber)->hadError) {
        nost_pushDynarr(vm, errs, nost_refAsFiber(vm, fiber)->err);
        NOST_POP_BLESSING(vm, bytecode);
        NOST_POP_BLESSING(vm, ast);
        return nost_noneVal();
    }

    NOST_POP_BLESSING(vm, bytecode);
    NOST_POP_BLESSING(vm, ast);
    return res;

}
