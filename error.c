
#include "vm.h"
#include "error.h"
#include "src.h"
#include "util.h"
#include "list.h"
#include "gc.h"

void nost_initError(nost_vm* vm, nost_error* error) {
    nost_initDynarr(vm, &error->pieces);
}

static void freeErrorPiece(nost_vm* vm, nost_errorPiece* piece) {
    switch(piece->type) {
        case NOST_PIECE_MESSAGE: {
            size_t msgLen = strlen(piece->as.message) + 2;
            NOST_FREE(vm, piece->as.message, msgLen);
            break;
        }
        case NOST_PIECE_VAL_REF:
        case NOST_PIECE_SRC_REF:
            break;
    }
}

void nost_freeError(nost_vm* vm, nost_error* error) {
    for(int i = 0; i < error->pieces.cnt; i++)
        freeErrorPiece(vm, &error->pieces.vals[i]);
    nost_freeDynarr(vm, &error->pieces);
}

void nost_addMessage(nost_vm* vm, nost_error* error, char* fmt, ...) {
    nost_errorPiece piece; 
    piece.type = NOST_PIECE_MESSAGE;

    va_list args;
    va_start(args, fmt); 
    va_list argsCopy;
    va_copy(argsCopy, args);

    size_t msgLen = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    piece.as.message = NOST_ALLOC(vm, msgLen + 1, "error message");
    vsnprintf(piece.as.message, msgLen, fmt, argsCopy);
    piece.as.message[msgLen] = '\0';
    va_end(argsCopy);

    nost_pushDynarr(vm, &error->pieces, piece);
}

void nost_addValRef(nost_vm* vm, nost_error* error, nost_val ref) {
    if(nost_isSrcObj(ref)) {
        nost_srcObj* srcObj = nost_asSrcObj(ref);
        nost_addSrcRef(vm, error, srcObj->begin, srcObj->end);
        return;
    }
    nost_errorPiece piece;
    piece.type = NOST_PIECE_VAL_REF;
    piece.as.valRef = ref;
    nost_pushDynarr(vm, &error->pieces, piece);
}

void nost_addSrcRef(struct nost_vm* vm, nost_error* error, nost_srcRef begin, nost_srcRef end) {
    nost_errorPiece piece;
    piece.type = NOST_PIECE_SRC_REF;
    piece.as.srcRef.begin = begin;
    piece.as.srcRef.end = end;
    nost_pushDynarr(vm, &error->pieces, piece);
}

void nost_addSrcPointRef(struct nost_vm* vm, nost_error* error, nost_srcRef point) {
    nost_addSrcRef(vm, error, point, point);
}

void nost_addValEndRef(struct nost_vm* vm, nost_error* error, nost_val ref) {
    if(nost_isSrcObj(ref)) {
        nost_srcObj* srcObj = nost_asSrcObj(ref);
        nost_addSrcPointRef(vm, error, srcObj->end);
        return;
    }
    nost_addValRef(vm, error, ref);
}

void nost_addListRef(struct nost_vm* vm, nost_error* error, nost_val list, int beginElem, int endElem) {

    nost_gcPause(vm);

    nost_val begin = nost_nth(vm, list, beginElem);
    nost_val end = nost_nth(vm, list, endElem);
    if(!nost_isSrcObj(begin) || !nost_isSrcObj(end)) {
        nost_addValRef(vm, error, list);
        return;
    }
    nost_srcObj* beginSrcObj = nost_asSrcObj(begin);
    nost_srcObj* endSrcObj = nost_asSrcObj(end);
    nost_addSrcRef(vm, error, beginSrcObj->begin, endSrcObj->end);

    nost_gcUnpause(vm);

}

void nost_doArgCntErrors(struct nost_vm* vm, nost_error* error, nost_val code, int args, const char** messages) {

    nost_val list = nost_unwrap(code);

    int argCnt = nost_len(vm, list) - 1;
    if(argCnt > args) {
        nost_addMessage(vm, error, "Unexpected arguments.");
        if(!nost_isNil(code))
            nost_addListRef(vm, error, list, args + 1, argCnt);
        return; 
    }

    if(messages[argCnt] == NULL)
        return;
    
    nost_addMessage(vm, error, "%s", messages[argCnt]);
    if(!nost_isNil(code))
        nost_addValEndRef(vm, error, code);

}

static void writeErrorPiece(nost_vm* vm, nost_str* str, nost_errorPiece* piece) {
    switch(piece->type) {
        case NOST_PIECE_MESSAGE: {
            nost_write(vm, str, "%s\n", piece->as.message);
            break;
        }
        case NOST_PIECE_VAL_REF: {
            nost_val ref = piece->as.valRef;
            nost_write(vm, str, "In generated code:\n");
            nost_writeVal(vm, str, ref); // TODO: this only shows the ref'd value without context. fix somehow.
            nost_write(vm, str, "\n");
            break;
        }
        case NOST_PIECE_SRC_REF: {

            nost_srcRef beginRef = piece->as.srcRef.begin; 
            nost_srcRef endRef = piece->as.srcRef.end; 
            nost_src* src = beginRef.src; 
            char* srcStr = src->src;

            if(src->name != NULL) {
                nost_write(vm, str, "  %s:\n", src->name);
            }

            int begin = beginRef.idx;
            int end = endRef.idx;
            int lineBegin = nost_findLineBegin(srcStr, begin);
            int lineEnd = nost_findLineEnd(srcStr, end);

            bool multiline = false;
            for(int i = lineBegin; i < lineEnd; i++)
                if(srcStr[i] == '\n')
                    multiline = true;

            // TODO: consider putting line info into src refs.
            int line = 1;
            for(int i = 0; i < lineBegin; i++)
                if(srcStr[i] == '\n')
                    line++;
                        
            if(!multiline) {

                int currOffset = nost_write(vm, str, "line %d |  ", line);
                int refOffset = currOffset;
                for(int i = lineBegin; i < lineEnd; i++) {
                    if(i == begin)
                        refOffset = currOffset;
                    if(srcStr[i] == '\t') {
                        nost_write(vm, str, "    ");
                        currOffset += 4;
                    } else {
                        nost_write(vm, str, "%c", srcStr[i]);
                        currOffset++;
                    }
                }
                nost_write(vm, str, "\n");
                for(int i = 0; i < refOffset; i++)
                    nost_write(vm, str, " ");
                nost_write(vm, str, "^");
                for(int i = 0; i < end - begin; i++)
                    nost_write(vm, str, "~");
                nost_write(vm, str, "\n");

            } else {

                int offset = nost_write(vm, str, "line %d ", line);
                nost_write(vm, str, "|  ");
                for(int i = lineBegin; i < lineEnd; i++) {
                    if(srcStr[i] == '\t') {
                        nost_write(vm, str, "    ");
                    } else if(srcStr[i] == '\n') {
                        nost_write(vm, str, "\n");
                        for(int i = 0; i < offset; i++)
                            nost_write(vm, str, " ");
                        nost_write(vm, str, "|  ");
                    } else {
                        nost_write(vm, str, "%c", srcStr[i]);
                    }
                }
                nost_write(vm, str, "\n");

            }
            
            break;
        }
    }
}

void nost_writeError(nost_vm* vm, nost_str* str, nost_error* error) {
    for(int i = 0; i < error->pieces.cnt; i++)
        writeErrorPiece(vm, str, &error->pieces.vals[i]);
}
