#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "util.h"

type type_to_array_type(type current) {
    DBUG_ENTER("type_to_array_type");
    DBUG_ASSERT((current != TP_void && current != TP_unknown), "Invalid type given");

    switch(current) {
        case TP_bool:
            DBUG_RETURN(TP_bool_array);
        case TP_int:
            DBUG_RETURN(TP_int_array);
        case TP_float:
            DBUG_RETURN(TP_float_array);
        default:
            DBUG_RETURN(current);
    }
}

void append_statement_to_tail(node* current_final_statement, node* new_final_statement) {
    DBUG_ENTER("append_statement_to_tail");

    DBUG_ASSERT((current_final_statement != NULL), "append_statement_to_tail called with NULL current final statement");
    DBUG_ASSERT((NODE_TYPE(current_final_statement) == N_statements), "append_statement_to_tail called with current_final_statement not being a statement type node");
    DBUG_ASSERT((NODE_TYPE(new_final_statement) == N_statements), "append_statement_to_tail called with new_final_statement not being a statement type node");

    while(STATEMENTS_NEXT(current_final_statement) != NULL) {
        current_final_statement = STATEMENTS_NEXT(current_final_statement);
    }
    // End found, link the new node to the end
    STATEMENTS_NEXT(current_final_statement) = new_final_statement;

    DBUG_VOID_RETURN;
}
