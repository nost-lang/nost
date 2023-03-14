
#include "value.h"
#include "sym.h"
#include "list.h"
#include "fn.h"
#include "pkg.h"
#include "src.h"
#include "str.h"

nost_val nost_nil() {
    return (nost_val){NOST_TYPE_NIL, {.num = 0}};
}

bool nost_isNil(nost_val val) {
    return val.type == NOST_TYPE_NIL;
}

nost_val nost_num(double x) {
    return (nost_val){NOST_TYPE_NUM, {.num = x}};
}

bool nost_isNum(nost_val val) {
    return val.type == NOST_TYPE_NUM;
}

double nost_asNum(nost_val val) {
    return val.as.num;
}

nost_val nost_objVal(nost_obj* obj) {
    return (nost_val){NOST_TYPE_OBJ, {.obj = obj}};
}

bool nost_isObj(nost_val val) {
    return val.type == NOST_TYPE_OBJ;
}

nost_obj* nost_asObj(nost_val val) {
    return val.as.obj;
}

static bool isObjType(nost_val val, nost_objType type) {
    if(!nost_isObj(val))
        return false;
    nost_obj* obj = nost_asObj(val);
    return obj->type == type;
}

bool nost_isSym(nost_val val) {
    return isObjType(val, NOST_OBJ_SYM);
}

nost_sym* nost_asSym(nost_val val) {
    return (nost_sym*)nost_asObj(val);
}

bool nost_isCons(nost_val val) {
    return isObjType(val, NOST_OBJ_CONS);
}

struct nost_cons* nost_asCons(nost_val val) {
    return (nost_cons*)nost_asObj(val);
}

bool nost_isFn(nost_val val) {
    return isObjType(val, NOST_OBJ_FN);
}

nost_fn* nost_asFn(nost_val val) {
    return (nost_fn*)nost_asObj(val);
}

bool nost_isNatfn(nost_val val) {
    return isObjType(val, NOST_OBJ_NATFN);
}

struct nost_natfn* nost_asNatfn(nost_val val) {
    return (nost_natfn*)nost_asObj(val);
}

bool nost_isPkg(nost_val val) {
    return isObjType(val, NOST_OBJ_PKG);
}

struct nost_pkg* nost_asPkg(nost_val val) {
    return (nost_pkg*)nost_asObj(val);
}

bool nost_isSrcObj(nost_val val) {
    return isObjType(val, NOST_OBJ_SRC_OBJ);
}

nost_srcObj* nost_asSrcObj(nost_val val) {
    return (nost_srcObj*)nost_asObj(val);
}

nost_val nost_unwrap(nost_val val) {
    if(nost_isSrcObj(val)) {
        nost_srcObj* srcObj = nost_asSrcObj(val);
        return srcObj->val;
    }
    return val;
}


nost_optVal nost_some(nost_val v) {
    nost_optVal opt;
    opt.nil = false;
    opt.val = v;
    return opt;
}

nost_optVal nost_none() {
    nost_optVal opt;
    opt.nil = true;
    return opt;
}

bool nost_truthy(nost_val v) {
    return !nost_isNil(v);
}

void nost_writeVal(nost_vm* vm, nost_str* str, nost_val val) {
    if(nost_isNil(val)) {
        nost_write(vm, str, "nil");
    } else if(nost_isNum(val)) {
        nost_write(vm, str, "%g", nost_asNum(val));
    } else if(nost_isSym(val)) {
        nost_write(vm, str, "%s", nost_asSym(val)->sym);
    } else if(nost_isCons(val)) {
        nost_cons* cons = nost_asCons(val);
        nost_val car = nost_car(vm, cons); 
        nost_val cdr = nost_cdr(vm, cons);
        nost_write(vm, str, "(");
        while(true) {
            nost_writeVal(vm, str, car);
            if(nost_isNil(cdr)) {
                nost_write(vm, str, ")");
                break;
            } else if(nost_isCons(cdr)) {
                nost_write(vm, str, " ");
                cons = nost_asCons(cdr);
                car = nost_car(vm, cons); 
                cdr = nost_cdr(vm, cons);
            } else {
                nost_write(vm, str, " . ");
                nost_writeVal(vm, str, cdr);
                nost_write(vm, str, ")");
                break;
            }
        } 
    } else if(nost_isSrcObj(val)) {
        nost_srcObj* srcObj = nost_asSrcObj(val);
        nost_writeVal(vm, str, srcObj->val);
    } else if(nost_isObj(val)) {
        nost_obj* obj = nost_asObj(val);
        nost_write(vm, str, "<%s>", nost_objTypenames[obj->type]);
    } else {
        nost_write(vm, str, "UNKNOWN VALUE");
    }
}

const char* nost_typename(nost_val val) {
    if(nost_isNil(val))
        return "nil";
    else if(nost_isNum(val))
        return "number";
    else if(nost_isObj(val))
        return nost_objTypenames[nost_asObj(val)->type];
    else 
        return "UNKNOWN VALUE TYPE";
}
