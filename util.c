
#include "util.h"

const char* nost_ordinalSuffix(int n) {
    if(n == 1)
        return "st";
    if(n == 2)
        return "nd";
    if(n == 3)
        return "rd";
    return "th";
}
