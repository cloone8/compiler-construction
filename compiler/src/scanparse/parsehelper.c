#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"
#include "myglobals.h"
#include "parsehelper.h"

size_t countNewlines(char* string, size_t length) {
    DBUG_ENTER("countNewlines");

    size_t linecount = 0;
    for(size_t i = 0; i < length; i++) {
        if(string[i] == '\n' && (i == 0 || string[i - 1] != '\r')) {
            linecount++;
        } else if(string[i] == '\r') {
            linecount++;
        }
    }

    DBUG_RETURN(linecount);
}

node* paramsGetNextNode(node* current) {
    DBUG_ENTER("paramsGetNextNode");

    switch (NODE_TYPE(current)) {
        case N_basicparameter:
            DBUG_RETURN(BASICPARAMETER_NEXT(current));
            break;

        case N_arrayparameter:
            DBUG_RETURN(ARRAYPARAMETER_NEXT(current));
            break;

        default:
            DBUG_ASSERT(FALSE, "Non-array parameter encountered in array parameter traversal function");
    }
    DBUG_RETURN(NULL);
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
    DBUG_ENTER("funcallExprsToParams");

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

    DBUG_RETURN(params_head);
}

int getIntFromText(char* text, int line, int col) {
    DBUG_ENTER("getIntFromText");

    long val = strtol(text, NULL, 10);
    if (val == 0) {
        // Conversion error
        if (errno == EINVAL) {
            CTIerror("Integer parsing error at line %d col %d!",  line, col);
            DBUG_RETURN(0);
        }

        // Out of range
        if (errno == ERANGE) {
            CTIerror("Integer out of range at line %d col %d!",  line, col);
            DBUG_RETURN(0);
        }
    }

    if(val < INT_MIN || val > INT_MAX) {
        CTIerror("Integer out of range at line %d col %d!",  line, col);
        DBUG_RETURN(0);
    }

    int toreturn = (int) val;

    if(toreturn < 0) {
        CTIerror("Integer out of range at line %d col %d!",  line, col);
        DBUG_RETURN(0);
    }

    DBUG_RETURN(toreturn);
}

float getFloatFromText(char* text, int line, int col) {
    DBUG_ENTER("getFloatFromText");

    float val = strtof(text, NULL);
    if (val == 0.0 || fabsf(val) == HUGE_VALF) {
        // Conversion error
        if (errno == EINVAL) {
            CTIerror("Float parsing error at line %d col %d!",  line, col);
            DBUG_RETURN(0.0f);
        }

        // Out of range
        if (errno == ERANGE) {
            CTIerror("Float out of range at line %d col %d!",  line, col);
            DBUG_RETURN(0.0f);
        }
    }
    if(val < 0) {
        CTIerror("Float out of range at line %d col %d!",  line, col);
        DBUG_RETURN(0.0f);
    }

    DBUG_RETURN(val);
}

/**
 * Interprets a GCC/Clang preprocessor directive.
 * Preprocessor directives have the following format:
 * # [linenum] "[filename]" [flags]
 * with each flag being on or more of the numbers 1, 2, 3 and 4 with each
 * number being seperated by a space.
 *
 * This function sets the global line, column and file names according to this
 * directive.
 * @param input: The complete preprocessor input directive as given by the lexer
 */
void interpretCPPstring(char* input) {
    DBUG_ENTER("interpretCPPstring");
    int inputlen = STRlen(input);

    // First, filter the first hash and space
    char* filestring_start = STRsubStr(input, 2, inputlen - 2);

    int toread = 0;
    char current_char = filestring_start[toread];

    // Read until the next space character
    while(current_char != ' ') {
        current_char = filestring_start[++toread];
    }

    // Get the current linenumber from the string
    char* linenr_substring = STRsubStr(filestring_start, 0, toread);

    // This is the final line number
    int current_line = getIntFromText(linenr_substring, global.line, global.col) - 1;

    // Go to the start of the filename
    char* filename_start = STRsubStr(filestring_start, toread + 2, inputlen - 4 - toread);

    toread = 0;
    current_char = filename_start[toread];

    // Read until the next space character
    while(current_char != '\"') {
        current_char = filename_start[++toread];
    }

    // This is the final filename
    char* filename_substr = STRsubStr(filename_start, 0, toread);

    global.line = current_line;
    myglobal.current_file = filename_substr;

    strlistAdd(myglobal.dangling_strings, filename_substr);

    MEMfree(filestring_start);
    MEMfree(linenr_substring);
    MEMfree(filename_start);

    DBUG_VOID_RETURN;
}
