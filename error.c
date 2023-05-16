
#include "error.h"
#include "src.h"
#include "gc.h"

void nost_initError(nost_error* error) {
    nost_initDynarr(&error->pieces);
}

static void freePiece(nost_vm* vm, nost_errorPiece* errorPiece) {
    switch(errorPiece->type) {
        case NOST_PIECE_MSG: {
            NOST_FREE(vm, errorPiece->as.msg);
            break;
        }
        case NOST_PIECE_SRC_REF: {
            NOST_FREE(vm, errorPiece->as.srcRef.line);
            break;
        }
        case NOST_PIECE_VAL_REF: {
            NOST_FREE(vm, errorPiece->as.valRef);
            break;
        }
    }
}

void nost_freeError(nost_vm* vm, nost_error* error) {
    for(int i = 0; i < error->pieces.cnt; i++)
        freePiece(vm, &error->pieces.vals[i]);
    nost_freeDynarr(vm, &error->pieces);
}

void nost_addMsg(nost_vm* vm, nost_error* error, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt); 
    va_list argsCopy;
    va_copy(argsCopy, args);
    
    size_t len = vsnprintf(NULL, 0, fmt, args);
    char* str = NOST_ALLOC(vm, len + 1);
    vsnprintf(str, len + 1, fmt, argsCopy);

    nost_errorPiece piece;
    piece.type = NOST_PIECE_MSG;
    piece.as.msg = str;
    nost_pushDynarr(vm, &error->pieces, piece);

    va_end(args);
    va_end(argsCopy); 
}

void nost_addSrcRef(nost_vm* vm, nost_error* error, nost_ref src, int start, int end) {
    nost_errorPiece piece;
    piece.type = NOST_PIECE_SRC_REF;
    
    size_t lineStart = start;
    while(lineStart != 0 && nost_refAsSrc(vm, src)->src[lineStart] != '\n')
        lineStart--;
    if(lineStart != 0)
        lineStart++;
    
    size_t lineEnd = end;
    while(lineEnd < nost_refAsSrc(vm, src)->len && nost_refAsSrc(vm, src)->src[lineEnd] != '\n')
        lineEnd++;

    char* line = NOST_ALLOC(vm, lineEnd - lineStart + 1);
    memcpy(line, nost_refAsSrc(vm, src)->src + lineStart, lineEnd - lineStart);
    line[lineEnd - lineStart] = '\0';

    piece.as.srcRef.line = line;

    piece.as.srcRef.offset = start - lineStart;
    piece.as.srcRef.len = end - start;
    piece.as.srcRef.multiline = false;
    for(int i = start; i < end; i++)
        if(nost_refAsSrc(vm, src)->src[i] == '\n')
            piece.as.srcRef.multiline = true;

    nost_pushDynarr(vm, &error->pieces, piece);
}

void nost_addValRef(nost_vm* vm, nost_error* error, nost_val val) {
    nost_ref ref = NOST_PUSH_BLESSING(vm, val);

    if(nost_refIsSrcObj(vm, ref)) {
        nost_ref src = NOST_PUSH_BLESSING(vm, nost_refAsSrcObj(vm, ref)->src);
        nost_addSrcRef(vm, error, src, nost_refAsSrcObj(vm, ref)->start, nost_refAsSrcObj(vm, ref)->end);
        NOST_POP_BLESSING(vm, src);
    } else {
        nost_errorPiece piece;
        piece.type = NOST_PIECE_VAL_REF;

        nost_str str;
        nost_initStr(&str);
        nost_writeVal(vm, &str, val);

        char* copy = NOST_ALLOC(vm, str.len + 1);
        memcpy(copy, str.str, str.len);
        copy[str.len] = '\0';
        piece.as.valRef = copy;

        nost_freeStr(vm, &str);

        nost_pushDynarr(vm, &error->pieces, piece);
    }

    NOST_POP_BLESSING(vm, ref);
}

static void writePiece(nost_vm* vm, nost_str* str, nost_errorPiece* piece) {
    switch(piece->type) {
        case NOST_PIECE_MSG: { 
            nost_writeStr(vm, str, "%s\n", piece->as.msg);
            break;
        }
        case NOST_PIECE_SRC_REF: {
            nost_writeStr(vm, str, "%s\n", piece->as.srcRef.line);
            if(!piece->as.srcRef.multiline) { // TODO: make this work well with \t
                for(int i = 0; i < piece->as.srcRef.offset; i++)
                    nost_writeStr(vm, str, " ");
                nost_writeStr(vm, str, "^");
                for(int i = 0; i < piece->as.srcRef.len - 1; i++)
                    nost_writeStr(vm, str, "~");
                nost_writeStr(vm, str, "\n");
            }
            break;
        }
        case NOST_PIECE_VAL_REF: {
            nost_writeStr(vm, str, "In generated value:\n\n%s\n", piece->as.valRef);
            break;
        }
    }
}

void nost_writeError(nost_vm* vm, nost_str* str, nost_error* error) {
    for(int i = 0; i < error->pieces.cnt; i++)
        writePiece(vm, str, &error->pieces.vals[i]); 
}
