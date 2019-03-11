#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "util.h"
#include "fun_symbol_table.h"

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

void append_statement_to_tail(node* block_to_append_to, node* new_final_statement) {
    DBUG_ENTER("append_statement_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_statement_to_tail called with NULL current final statement");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_statements), "append_statement_to_tail called with block_to_append_to not being a statement type node");
    DBUG_ASSERT((NODE_TYPE(new_final_statement) == N_statements), "append_statement_to_tail called with new_final_statement not being a statement type node");

    node* current_final_statement = block_to_append_to;
    while(STATEMENTS_NEXT(current_final_statement) != NULL) {
        current_final_statement = STATEMENTS_NEXT(current_final_statement);
    }
    // End found, link the new node to the end
    STATEMENTS_NEXT(current_final_statement) = new_final_statement;

    DBUG_VOID_RETURN;
}

void append_declaration_to_tail(node *block_to_append_to, node *new_final_declaration) {
    DBUG_ENTER("append_declaration_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_declaration_to_tail called with NULL current final declaration");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_declaration), "append_declaration_to_tail called with block_to_append_to not being a declaration type node");
    DBUG_ASSERT((NODE_TYPE(new_final_declaration) == N_declaration), "append_declaration_to_tail called with new_final_declaration not being a declaration type node");

    node* current_final_declaration = block_to_append_to;
    while (DECLARATION_NEXT(current_final_declaration) != NULL) {
        current_final_declaration = DECLARATION_NEXT(current_final_declaration);
    }
    // End found, link the new node to the end
    DECLARATION_NEXT(current_final_declaration) = new_final_declaration;

    DBUG_VOID_RETURN;
}

fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find) {
    DBUG_ENTER("fun_is_in_scope");
    fun_symbol_table* current_table = deepest_table;

    while(current_table != NULL) {
        fun_symbol_table_entry* result = fun_symbol_table_get_name(current_table, to_find);

        if(result != NULL) {
            DBUG_PRINT("fun_is_in_scope", ("Function %s is in scope", to_find));
            DBUG_RETURN(result);
        }

        DBUG_PRINT("fun_is_in_scope", ("Function %s not found, checking next table"));

        current_table = fun_symbol_table_move_up(current_table, 1);
    }

    DBUG_PRINT("fun_is_in_scope", ("Function %s is NOT in scope", to_find));
    DBUG_RETURN(NULL);
}

symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find) {
    DBUG_ENTER("var_is_in_scope");
    symbol_table* current_table = deepest_table;

    while(current_table != NULL) {
        symbol_table_entry* result = symbol_table_get_name(current_table, to_find);

        if(result != NULL) {
            DBUG_PRINT("var_is_in_scope", ("Variable %s is in scope", to_find));
            DBUG_RETURN(result);
        }

        DBUG_PRINT("var_is_in_scope", ("Variable %s not found, checking next table"));

        current_table = symbol_table_move_up(current_table, 1);
    }

    DBUG_PRINT("is_in_scope", ("Variable %s is NOT in scope", to_find));
    DBUG_RETURN(NULL);
}
