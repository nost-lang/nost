
#ifndef NOST_COMMON_H
#define NOST_COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include "config.h"

#define NOST_XSTR(x) #x
#define NOST_STRINGIFY(x) NOST_XSTR(x) 
#define NOST_LOC() __FILE__ ":" NOST_STRINGIFY(__LINE__)

#ifdef NOST_STRICT_MODE

#define NOST_ASSERT(cond, message) \
    do { \
        if(!(cond)) { \
            printf("ASSERTION FAILED AT " NOST_LOC() "!\n" message "\n"); \
            raise(SIGABRT); \
            exit(1); \
        } \
    } while(0);

#else

#define NOST_ASSERT(cond, message) (void)(cond);

#endif

#endif
