#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "parsehelper.h"

size_t countNewlines(char* string, size_t length) {
    size_t linecount = 0;
    for(int i = 0; i < length; i++) {
        if(string[i] == '\n' && (i == 0 || string[i - 1] != '\r')) {
            linecount++;
        } else if(string[i] == '\r') {
            linecount++;
        }
    }

    return linecount;
}

node* paramsGetNextNode(node* current) {
    switch (NODE_TYPE(current)) {
        case N_basicparameter:
            return BASICPARAMETER_NEXT(current);
            break;

        case N_arrayparameter:
            return ARRAYPARAMETER_NEXT(current);
            break;

        default:
            DBUG_ASSERT(FALSE, "Non-array parameter encountered in array parameter traversal function");
    }
    return NULL;
}

void paramsSetNextNode(node* current, node* newNext) {
    switch (NODE_TYPE(current)) {
        case N_basicparameter:
            BASICPARAMETER_NEXT(current) = newNext;
            break;

        case N_arrayparameter:
            ARRAYPARAMETER_NEXT(current) = newNext;
            break;

        default:
            DBUG_ASSERT(FALSE, "Non-array parameter encountered in array parameter traversal function");
    }
}

node* funvardecsGetNextNode(node* current) {
    switch (NODE_TYPE(current)) {
        case N_basicparameter:
            return BASICPARAMETER_NEXT(current);
            break;

        case N_arrayparameter:
            return ARRAYPARAMETER_NEXT(current);
            break;

        default:
            DBUG_ASSERT(FALSE, "Non-vardec node encountered in function vardecs traversal function");
    }
    return NULL;
}

void funvardecsSetNextNode(node* current, node* newNext) {
    switch (NODE_TYPE(current)) {
        case N_basicparameter:
            BASICPARAMETER_NEXT(current) = newNext;
            break;

        case N_arrayparameter:
            ARRAYPARAMETER_NEXT(current) = newNext;
            break;

        default:
            DBUG_ASSERT(FALSE, "Non-vardec node encountered in function vardecs traversal function");
    }
}
