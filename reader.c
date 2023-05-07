
#include "reader.h"
#include "src.h"
#include "gc.h"
#include "sym.h"
#include "list.h"

void nost_initReader(nost_reader* reader, nost_ref src) {
    reader->src = src;
    reader->eof = false;
    reader->curr = 0;
}

static bool whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r'; 
}

static bool numeric(char c) {
    return c >= '0' && c <= '9';
}

static char charAt(nost_vm* vm, nost_reader* reader, int idx) {
    return nost_refAsSrc(vm, reader->src)->src[idx];
}

static char curr(nost_vm* vm, nost_reader* reader) {
    return charAt(vm, reader, reader->curr);
}

static void advance(nost_vm* vm, nost_reader* reader) {
    if(reader->eof)
        return;
    reader->curr++;
    if(curr(vm, reader) == '\0')
        reader->eof = true;
}

static void skipWhitespace(nost_vm* vm, nost_reader* reader) {
    char currChar = curr(vm, reader);
    while(whitespace(currChar)) {
        advance(vm, reader);
        currChar = curr(vm, reader);
    }
}

static nost_error* makeError(nost_vm* vm, nost_errors* errors) {
    nost_error error;
    nost_initError(&error);
    nost_addMsg(vm, &error, "Reading Error.");
    nost_pushDynarr(vm, errors, error);
    nost_error* res = &errors->vals[errors->cnt - 1];
    return res;
}

static nost_val read(nost_vm* vm, nost_reader* reader, nost_errors* errors) {
    if(reader->eof)
        return nost_noneVal(); 

    if(curr(vm, reader) == '(') {
        advance(vm, reader);
        nost_ref list = nost_pushBlessing(vm, nost_nilVal());
        nost_ref tail = nost_pushBlessing(vm, nost_nilVal());
        while(true) {
            if(reader->eof || curr(vm, reader) == ')')
                break;
            nost_val next = nost_read(vm, reader, errors);
            nost_ref nextRef = nost_pushBlessing(vm, next);
            nost_val cons = nost_makeCons(vm); 
            nost_initCons(cons, nost_getRef(vm, nextRef), nost_nilVal());
            if(nost_refIsNil(vm, list)) {
                nost_setRef(vm, list, cons);
                nost_setRef(vm, tail, cons);
            } else {
                nost_setCdr(vm, nost_getRef(vm, tail), cons);
                nost_setRef(vm, tail, cons);
            }
            nost_popBlessing(vm);
        }
        if(curr(vm, reader) != ')') {
            nost_error* err = makeError(vm, errors);
            nost_addMsg(vm, err, "Expected closing bracket.");
        } else {
            advance(vm, reader);
        }
        nost_val res = nost_getRef(vm, list);
        nost_popBlessing(vm);
        nost_popBlessing(vm);
        return res;
    }
    
    int start = reader->curr;
    while(!reader->eof && !whitespace(curr(vm, reader)) && curr(vm, reader) != '(' && curr(vm, reader) != ')') {
        advance(vm, reader);
    }
    
    // TODO: add decimal support
    bool allDigits = true;
    for(int i = start; i < reader->curr; i++)
        if(!numeric(charAt(vm, reader, i)))
            allDigits = false;
    
    if(!allDigits) {
        if(nost_refAsSrc(vm, reader->src)->len >= (size_t)start + 3 && memcmp(nost_refAsSrc(vm, reader->src)->src + start, "nil", 3) == 0) {
            return nost_nilVal();
        }
        nost_val res = nost_makeSymWithLen(vm, reader->curr - start); 
        nost_initSym(res, nost_refAsSrc(vm, reader->src)->src + start);
        return res;
    } else {
        double value = 0;
        for(int i = start; i < reader->curr; i++) {
            value *= 10;
            value += charAt(vm, reader, i) - '0';
        }
        return nost_numVal(value); 
    }
}

nost_val nost_read(nost_vm* vm, nost_reader* reader, nost_errors* errors) {

    skipWhitespace(vm, reader);

    int start = reader->curr;
    nost_val val = read(vm, reader, errors); 
    int end = reader->curr;

    if(nost_isNone(val))
        return val;

    nost_ref valRef = nost_pushBlessing(vm, val);
    nost_val res = nost_makeSrcObj(vm, reader->src, valRef, start, end); 
    nost_popBlessing(vm);

    return res;
}

void nost_freeReader(nost_vm* vm, nost_reader* reader) {
    (void)vm;
    (void)reader;
}
