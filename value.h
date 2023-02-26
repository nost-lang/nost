
#ifndef NOST_VALUE_H
#define NOST_VALUE_H

#include "common.h"
#include "obj.h"

typedef enum {
    NOST_TYPE_NIL,
    NOST_TYPE_NUM,
    NOST_TYPE_OBJ
} nost_valType;

typedef struct {
    nost_valType type;
    union {
        double num;
        nost_obj* obj;
    } as;
} nost_val;

nost_val nost_nil();
bool nost_isNil(nost_val val);

nost_val nost_num(double x);
bool nost_isNum(nost_val val);
double nost_asNum(nost_val val);

nost_val nost_objVal(nost_obj* obj);
bool nost_isObj(nost_val val);
nost_obj* nost_asObj(nost_val val);

struct nost_sym;
bool nost_isSym(nost_val val);
struct nost_sym* nost_asSym(nost_val val);

struct nost_cons;
bool nost_isCons(nost_val val);
struct nost_cons* nost_asCons(nost_val val);

struct nost_fn;
bool nost_isFn(nost_val val);
struct nost_fn* nost_asFn(nost_val val);

struct nost_natfn;
bool nost_isNatfn(nost_val val);
struct nost_natfn* nost_asNatfn(nost_val val);

struct nost_pkg;
bool nost_isPkg(nost_val val);
struct nost_pkg* nost_asPkg(nost_val val);

typedef struct {
    bool nil;
    nost_val val;
} nost_optVal;

nost_optVal nost_some(nost_val v);
nost_optVal nost_none();

bool nost_truthy(nost_val v);

#endif
