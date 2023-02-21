
#include "value.h"
#include "sym.h"
#include "list.h"
#include "fn.h"

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
