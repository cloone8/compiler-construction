/**
 * This header contains functions for appending blocks of nodes to the end of another block of nodes.
 * For example:
 * 1 -> 2 -> 3 -> 4 -> 5
 * Calling one of these append functions to any of the five nodes in the block, results in
 * 1 -> 2 -> 3 -> 4 -> 5 -> new_node
 */

#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "util.h"
#include "node_appends.h"

void append_statement_to_tail(node* block_to_append_to, node* statement_to_append) {
    DBUG_ENTER("append_statement_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_statement_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((statement_to_append != NULL), "append_statement_to_tail called with NULL statement_to_append");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_statements), "append_statement_to_tail called with block_to_append_to not being a statement type node");
    DBUG_ASSERT((NODE_TYPE(statement_to_append) == N_statements), "append_statement_to_tail called with statement_to_append not being a statement type node");

    node* current_final_statement = block_to_append_to;
    while(STATEMENTS_NEXT(current_final_statement) != NULL) {
        current_final_statement = STATEMENTS_NEXT(current_final_statement);
    }
    // End found, link the new node to the end
    STATEMENTS_NEXT(current_final_statement) = statement_to_append;

    DBUG_VOID_RETURN;
}

void append_declaration_to_tail(node *block_to_append_to, node *declaration_to_append) {
    DBUG_ENTER("append_declaration_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_declaration_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((declaration_to_append != NULL), "append_declaration_to_tail called with NULL declaration_to_append");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_declaration), "append_declaration_to_tail called with block_to_append_to not being a declaration type node");
    DBUG_ASSERT((NODE_TYPE(declaration_to_append) == N_declaration), "append_declaration_to_tail called with declaration_to_append not being a declaration type node");

    node* current_final_declaration = block_to_append_to;
    while (DECLARATION_NEXT(current_final_declaration) != NULL) {
        current_final_declaration = DECLARATION_NEXT(current_final_declaration);
    }
    // End found, link the new node to the end
    DECLARATION_NEXT(current_final_declaration) = declaration_to_append;

    DBUG_VOID_RETURN;
}

void append_functionvardeclaration_to_tail(node *block_to_append_to, node *declaration_to_append) {
    DBUG_ENTER("append_functionvardeclaration_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_functionvardeclaration_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((declaration_to_append != NULL), "append_functionvardeclaration_to_tail called with NULL declaration_to_append");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_functionvardeclaration), "append_functionvardeclaration_to_tail called with block_to_append_to not being a functionvardeclaration type node");
    DBUG_ASSERT((NODE_TYPE(declaration_to_append) == N_functionvardeclaration), "append_functionvardeclaration_to_tail called with declaration_to_append not being a functionvardeclaration type node");

    node* current_final_declaration = block_to_append_to;
    while (FUNCTIONVARDECLARATION_NEXT(current_final_declaration) != NULL) {
        current_final_declaration = FUNCTIONVARDECLARATION_NEXT(current_final_declaration);
    }
    // End found, link the new node to the end
    FUNCTIONVARDECLARATION_NEXT(current_final_declaration) = declaration_to_append;

    DBUG_VOID_RETURN;
}

void append_exprarray_to_tail(node *block_to_append_to, node *exprarray_to_append) {
    DBUG_ENTER("append_exprarray_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_exprarray_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((exprarray_to_append != NULL), "append_exprarray_to_tail called with NULL exprarray_to_append");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_exprarray), "append_exprarray_to_tail called with block_to_append_to not being a exprarray type node");
    DBUG_ASSERT((NODE_TYPE(exprarray_to_append) == N_exprarray), "append_exprarray_to_tail called with exprarray_to_append not being a exprarray type node");

    node* current_final_exprarray = block_to_append_to;
    while (EXPRARRAY_NEXT(current_final_exprarray) != NULL) {
        current_final_exprarray = EXPRARRAY_NEXT(current_final_exprarray);
    }
    // End found, link the new node to the end
    EXPRARRAY_NEXT(current_final_exprarray) = exprarray_to_append;

    DBUG_VOID_RETURN;
}

static inline node* get_next_funparam(node* funparam) {
    DBUG_ENTER("get_next_funparam");
    DBUG_ASSERT((funparam != NULL), "funparam is NULL!");
    node* toreturn = NULL;

    switch(NODE_TYPE(funparam)) {
        case N_basicparameter:
            toreturn = BASICPARAMETER_NEXT(funparam);
            break;
        case N_arrayparameter:
            toreturn = ARRAYPARAMETER_NEXT(funparam);
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid get_next_funparam type for funparam!");
            break;
    }

    DBUG_RETURN(toreturn);
}

static inline void set_next_funparam(node* funparam, node* toset) {
    DBUG_ENTER("set_next_funparam");
    DBUG_ASSERT((funparam != NULL), "funparam is NULL!");
    DBUG_ASSERT((toset == NULL || NODE_TYPE(toset) == N_basicparameter || NODE_TYPE(toset) == N_arrayparameter), "Invalid type for toset!");

    switch(NODE_TYPE(funparam)) {
        case N_basicparameter:
            BASICPARAMETER_NEXT(funparam) = toset;
            break;
        case N_arrayparameter:
            ARRAYPARAMETER_NEXT(funparam) = toset;
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid type for funparam!");
            break;
    }

    DBUG_VOID_RETURN;
}

void append_functionparameter_to_tail(node *block_to_append_to, node *funparam_to_append) {
    DBUG_ENTER("append_functionparameter_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_functionparameter_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((funparam_to_append != NULL), "append_functionparameter_to_tail called with NULL funparam_to_append");

    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_basicparameter || NODE_TYPE(block_to_append_to) == N_arrayparameter),
        "append_functionparameter_to_tail called with block_to_append_to not being a basicparameter or arrayparameter type node");
    DBUG_ASSERT((NODE_TYPE(funparam_to_append) == N_basicparameter || NODE_TYPE(funparam_to_append) == N_arrayparameter),
        "append_functionparameter_to_tail called with funparam_to_append not being a basicparameter or arrayparameter type node");

    node* current_final_basicparam = block_to_append_to;

    while (get_next_funparam(current_final_basicparam) != NULL) {
        current_final_basicparam = get_next_funparam(current_final_basicparam);
    }
    // End found, link the new node to the end
    set_next_funparam(current_final_basicparam, funparam_to_append);

    DBUG_VOID_RETURN;
}

void append_funcallparam_to_tail(node *block_to_append_to, node *funcallparam_to_append) {
    DBUG_ENTER("append_funcallparam_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_funcallparam_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((funcallparam_to_append != NULL), "append_funcallparam_to_tail called with NULL funcallparam_to_append");

    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_funcallparameter),
        "append_funcallparam_to_tail called with block_to_append_to not being a funcallparam type node");
    DBUG_ASSERT((NODE_TYPE(funcallparam_to_append) == N_funcallparameter),
        "append_funcallparam_to_tail called with funcallparam_to_append not being a funcallparam type node");

    node* current_final_funcallparam = block_to_append_to;

    while (FUNCALLPARAMETER_NEXT(current_final_funcallparam) != NULL) {
        current_final_funcallparam = FUNCALLPARAMETER_NEXT(current_final_funcallparam);
    }
    // End found, link the new node to the end
    FUNCALLPARAMETER_NEXT(current_final_funcallparam) = funcallparam_to_append;

    DBUG_VOID_RETURN;
}
