#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "util.h"
#include "fun_symbol_table.h"
#include "str.h"

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

type array_type_to_type(type current) {
    DBUG_ENTER("array_type_to_type");
    DBUG_ASSERT((current != TP_void && current != TP_unknown), "Invalid type given");

    switch(current) {
        case TP_bool_array:
            DBUG_RETURN(TP_bool);
        case TP_int_array:
            DBUG_RETURN(TP_int);
        case TP_float_array:
            DBUG_RETURN(TP_float);
        default:
            DBUG_RETURN(current);
    }
}

void append_statement_to_tail(node* block_to_append_to, node* new_final_statement) {
    DBUG_ENTER("append_statement_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_statement_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((new_final_statement != NULL), "append_statement_to_tail called with NULL new_final_statement");
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

    DBUG_ASSERT((block_to_append_to != NULL), "append_declaration_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((new_final_declaration != NULL), "append_declaration_to_tail called with NULL new_final_declaration");
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

void append_functionvardeclaration_to_tail(node *block_to_append_to, node *new_final_declaration) {
    DBUG_ENTER("append_functionvardeclaration_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_functionvardeclaration_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((new_final_declaration != NULL), "append_functionvardeclaration_to_tail called with NULL new_final_declaration");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_functionvardeclaration), "append_functionvardeclaration_to_tail called with block_to_append_to not being a functionvardeclaration type node");
    DBUG_ASSERT((NODE_TYPE(new_final_declaration) == N_functionvardeclaration), "append_functionvardeclaration_to_tail called with new_final_declaration not being a functionvardeclaration type node");

    node* current_final_declaration = block_to_append_to;
    while (FUNCTIONVARDECLARATION_NEXT(current_final_declaration) != NULL) {
        current_final_declaration = FUNCTIONVARDECLARATION_NEXT(current_final_declaration);
    }
    // End found, link the new node to the end
    FUNCTIONVARDECLARATION_NEXT(current_final_declaration) = new_final_declaration;

    DBUG_VOID_RETURN;
}

void append_exprarray_to_tail(node *block_to_append_to, node *new_final_exprarray) {
    DBUG_ENTER("append_exprarray_to_tail");

    DBUG_ASSERT((block_to_append_to != NULL), "append_exprarray_to_tail called with NULL block_to_append_to");
    DBUG_ASSERT((new_final_exprarray != NULL), "append_exprarray_to_tail called with NULL new_final_exprarray");
    DBUG_ASSERT((NODE_TYPE(block_to_append_to) == N_exprarray), "append_exprarray_to_tail called with block_to_append_to not being a exprarray type node");
    DBUG_ASSERT((NODE_TYPE(new_final_exprarray) == N_exprarray), "append_exprarray_to_tail called with new_final_exprarray not being a exprarray type node");

    node* current_final_exprarray = block_to_append_to;
    while (EXPRARRAY_NEXT(current_final_exprarray) != NULL) {
        current_final_exprarray = EXPRARRAY_NEXT(current_final_exprarray);
    }
    // End found, link the new node to the end
    EXPRARRAY_NEXT(current_final_exprarray) = new_final_exprarray;

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

static void print_exprarray_rec(node* exprs, size_t level) {
    DBUG_ENTER("print_exprarray_red");
    DBUG_ASSERT((NODE_TYPE(exprs) == N_exprarray), "Wrong node type given!");
    DBUG_PRINT("print_exprarray_rec", ("Entering exprarray (level %zu)\n", level));

    if(NODE_TYPE(EXPRARRAY_CURRENT(exprs)) == N_exprarray) {
        print_exprarray_rec(EXPRARRAY_CURRENT(exprs), level + 1);
    }

    DBUG_PRINT("print_exprarray_rec", ("Leaving exprarray (level %zu)\n", level));

    if(EXPRARRAY_NEXT(exprs) != NULL) {
        print_exprarray_rec(EXPRARRAY_NEXT(exprs), level);
    }

    DBUG_VOID_RETURN;
}

void print_exprarray(node* exprs) {
    DBUG_ENTER("print_exprarray");
    DBUG_ASSERT((NODE_TYPE(exprs) == N_exprarray), "Wrong node type given!");
    print_exprarray_rec(exprs, 0);
    DBUG_VOID_RETURN;
}

char* print_type(type to_print) {
    DBUG_ENTER("print_type");

    char* result;

    switch (to_print) {
        case TP_bool:
            result = "BOOL";
            break;
        case TP_bool_array:
            result = "BOOL_ARRAY";
            break;
        case TP_int:
            result = "INT";
            break;
        case TP_int_array:
            result = "INT_ARRAY";
            break;
        case TP_float:
            result = "FLOAT";
            break;
        case TP_float_array:
            result = "FLOAT_ARRAY";
            break;
        default:
            result = "UNKNOWN";
            break;
    }

    DBUG_RETURN(result);
}

char* print_nodetype(nodetype nt) {
    DBUG_ENTER("print_nodetype");
    char* toreturn = NULL;

    switch(nt) {
        case N_undefined:
            toreturn = STRcpy("undefined");
            break;
        case N_program:
            toreturn = STRcpy("program");
            break;
        case N_declaration:
            toreturn = STRcpy("declaration");
            break;
        case N_function:
            toreturn = STRcpy("function");
            break;
        case N_basicparameter:
            toreturn = STRcpy("basicparameter");
            break;
        case N_arrayparameter:
            toreturn = STRcpy("arrayparameter");
            break;
        case N_arraydimensionvars:
            toreturn = STRcpy("arraydimensionvars");
            break;
        case N_functiondefinition:
            toreturn = STRcpy("functiondefinition");
            break;
        case N_functionbody:
            toreturn = STRcpy("functionbody");
            break;
        case N_functionvardeclaration:
            toreturn = STRcpy("functionvardeclaration");
            break;
        case N_localfunction:
            toreturn = STRcpy("localfunction");
            break;
        case N_statements:
            toreturn = STRcpy("statements");
            break;
        case N_basicvardeclaration:
            toreturn = STRcpy("basicvardeclaration");
            break;
        case N_arrayvardeclaration:
            toreturn = STRcpy("arrayvardeclaration");
            break;
        case N_exprarray:
            toreturn = STRcpy("exprarray");
            break;
        case N_globaldefinition:
            toreturn = STRcpy("globaldefintion");
            break;
        case N_for:
            toreturn = STRcpy("for");
            break;
        case N_while:
            toreturn = STRcpy("while");
            break;
        case N_dowhile:
            toreturn = STRcpy("dowhile");
            break;
        case N_global:
            toreturn = STRcpy("global");
            break;
        case N_error:
            toreturn = STRcpy("error");
            break;
        case N_funcall:
            toreturn = STRcpy("funcall");
            break;
        case N_funcallparameter:
            toreturn = STRcpy("funcallparameter");
            break;
        case N_return:
            toreturn = STRcpy("return");
            break;
        case N_branch:
            toreturn = STRcpy("branch");
            break;
        case N_expr:
            toreturn = STRcpy("expr");
            break;
        case N_monop:
            toreturn = STRcpy("monop");
            break;
        case N_constant:
            toreturn = STRcpy("constant");
            break;
        case N_floatconst:
            toreturn = STRcpy("floatconst");
            break;
        case N_intconst:
            toreturn = STRcpy("intconst");
            break;
        case N_boolconst:
            toreturn = STRcpy("boolconst");
            break;
        case N_cast:
            toreturn = STRcpy("cast");
            break;
        case N_varread:
            toreturn = STRcpy("varread");
            break;
        case N_varassign:
            toreturn = STRcpy("varassign");
            break;
        case N_arrayvarread:
            toreturn = STRcpy("arrayvarread");
            break;
        case N_arrayvarassign:
            toreturn = STRcpy("arrayvarassign");
            break;
        case N_binop:
            toreturn = STRcpy("binop");
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid nodetype given!");
    }
    DBUG_RETURN(toreturn);
}
