
#ifndef NOST_PARSER_H
#define NOST_PARSER_H

#include "vm.h"
#include "src.h"

typedef struct {
    nost_src* src;
    int curr; 
    nost_dynarr(nost_error) errors;
} nost_parser; 

void nost_initParser(nost_vm* vm, nost_parser* parser, nost_src* src);
void nost_freeParser(nost_vm* vm, nost_parser* parser);
nost_optVal nost_parse(nost_vm* vm, nost_parser* parser);

#endif
