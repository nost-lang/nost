
#include "parser.h"
#include "sym.h"
#include "list.h"
#include "gc.h"

void nost_initParser(nost_vm* vm, nost_parser* parser, nost_src* src) {
    parser->src = src;
    parser->curr = 0;
    nost_initDynarr(vm, &parser->errors);
}

void nost_freeParser(nost_vm* vm, nost_parser* parser) {
    nost_freeDynarr(vm, &parser->errors);
}

static bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool isSymbreaking(char c) {
    return isWhitespace(c) || c == '(' || c == ')' || c == '\0';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static char curr(nost_parser* parser) {
    return parser->src->src[parser->curr];
}

static bool atEnd(nost_parser* parser) {
    return curr(parser) == '\0';
}

static void advance(nost_parser* parser) {
    if(atEnd(parser))
        return;
    parser->curr++;
}

static void skipWhitespace(nost_parser* parser) {
    while(isWhitespace(curr(parser)))
        advance(parser);
}

static void error(nost_vm* vm, nost_parser* parser, nost_error err) {
    nost_pushDynarr(vm, &parser->errors, err);
}

static nost_optVal parse(nost_vm* vm, nost_parser* parser) {
    skipWhitespace(parser);
    if(atEnd(parser))
        return nost_none();

    if(curr(parser) == '(') {
        advance(parser);

        nost_dynarr(nost_val) elems;
        nost_initDynarr(vm, &elems);
        skipWhitespace(parser);
        while(curr(parser) != ')') {
            nost_optVal optExpr = parse(vm, parser);
            if(!optExpr.nil) {
                nost_pushDynarr(vm, &elems, optExpr.val);
            } else {
                nost_error err;
                nost_initError(&err, "Expected ).");
                error(vm, parser, err);
                return nost_some(nost_nil());
            } 
            skipWhitespace(parser);
        }
        advance(parser);
        
        nost_val res = nost_list(vm, elems.cnt, elems.vals);
        nost_freeDynarr(vm, &elems);
        return nost_some(res);
    }
    
    int start = parser->curr;

    bool isNum = isDigit(curr(parser));
    double numVal = 0;
    double currDecimal = 0.1;
    bool wholePart = true;

    while(!isSymbreaking(curr(parser))) {

        if(isDigit(curr(parser)) && isNum) {
            int digit = curr(parser) - '0';
            if(wholePart) {
                numVal *= 10; 
                numVal += digit;
            } else {
                numVal += currDecimal * digit;
                currDecimal /= 10; 
            }
        } else if(curr(parser) == '.' && wholePart) {
            wholePart = false;
        } else {
            isNum = false;
        }

        advance(parser);
    }

    if(isNum)
        return nost_some(nost_num(numVal));

    return nost_some(nost_makeSym(vm, parser->src->src + start, parser->curr - start));
}

nost_optVal nost_parse(nost_vm* vm, nost_parser* parser) {
    nost_gcPause(vm);
    nost_optVal parsedCode = parse(vm, parser);
    nost_gcUnpause(vm);
    if(!parsedCode.nil)
        nost_blessVal(vm, parsedCode.val);        
    return parsedCode;
}
