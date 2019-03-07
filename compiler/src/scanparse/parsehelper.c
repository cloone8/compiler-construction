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
    for(size_t i = 0; i < length; i++) {
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

node* funcallExprsToParams(node *exprarray) {
    // Copy all expressions into a funcallparameter node
    node* current_expr = exprarray;

    node* params_head = NULL;
    node* params_tail = NULL;

    while(current_expr != NULL) {
        node* new_param = TBmakeFuncallparameter(EXPRARRAY_CURRENT(current_expr), NULL);

        if(params_head == NULL) {
            params_head = new_param;
        }

        if(params_tail != NULL) {
            FUNCALLPARAMETER_NEXT(params_tail) = new_param;
        }

        params_tail = new_param;

        node* previous_node = current_expr;
        current_expr = EXPRARRAY_NEXT(current_expr);

        EXPRARRAY_CURRENT(previous_node) = NULL;
        EXPRARRAY_NEXT(previous_node) = NULL;
        FREEexprarray(previous_node, NULL);
    }

    return params_head;
}
