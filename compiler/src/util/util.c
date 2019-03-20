#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "util.h"
#include "fun_symbol_table.h"
#include "str.h"

/**
 * Converts a basic type to its respective array type.
 * If the given type is not a basic type, does nothing and
 * just returns the parameter
 * @param current: The type to convert
 * @return: The converted type
 */
type type_to_array_type(type current) {
    DBUG_ENTER("type_to_array_type");

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

/**
 * Converts an array type to its respective basic type.
 * If the given type is not an array type, does nothing and
 * just returns the parameter
 * @param current: The type to convert
 * @return: The converted type
 */
type array_type_to_type(type current) {
    DBUG_ENTER("array_type_to_type");

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

/**
 * Given the name of a function, tries to find it in the current symbol table
 * and all parent tables.
 * @param deepest_table: The table representing the current deepest scope.
 * @param to_find: The name of the function to find.
 * @return The table entry of the function if it was found and is in scope,
 *         NULL otherwise.
 */
fun_symbol_table_entry* fun_is_in_scope(fun_symbol_table* deepest_table, char* to_find) {
    DBUG_ENTER("fun_is_in_scope");
    DBUG_ASSERT((deepest_table != NULL), "deepest_table cannot be NULL!");
    DBUG_ASSERT((to_find != NULL), "to_find cannot be NULL!");

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

/**
 * Given the name of a variable, tries to find it in the current symbol table
 * and all parent tables.
 * @param deepest_table: The table representing the current deepest scope.
 * @param to_find: The name of the variable to find.
 * @return The table entry of the variable if it was found and is in scope,
 *         NULL otherwise.
 */
symbol_table_entry* var_is_in_scope(symbol_table* deepest_table, char* to_find) {
    DBUG_ENTER("var_is_in_scope");
    DBUG_ASSERT((deepest_table != NULL), "deepest_table cannot be NULL!");
    DBUG_ASSERT((to_find != NULL), "to_find cannot be NULL!");

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


// As the function is only used when DBUG_OFF is not defined, we don't have
// to compile it if it is.
#ifndef DBUG_OFF

    /**
     * The actual recursive function for print_exprarrray
     * @param exprs: The exprarray to print
     * @param level: The current nesting level of this exprarray
     * @see print_exprarray
     */
    static void print_exprarray_rec(node* exprs, size_t level) {
        DBUG_ENTER("print_exprarray_rec");
        DBUG_ASSERT((NODE_TYPE(exprs) == N_exprarray), "Wrong node type given!");



        // If this EXPRARRAY contains another EXPRARRAY, go deeper
        if(NODE_TYPE(EXPRARRAY_CURRENT(exprs)) == N_exprarray) {
            DBUG_PRINT("print_exprarray_rec", ("Entering exprarray (level %zu)", level));
            print_exprarray_rec(EXPRARRAY_CURRENT(exprs), level + 1);
            DBUG_PRINT("print_exprarray_rec", ("Leaving exprarray (level %zu)", level));
        } else {
            DBUG_PRINT("print_exprarray_rec", ("Deepest exprarray (level %zu)", level));
        }

        if(EXPRARRAY_NEXT(exprs) != NULL) {
            print_exprarray_rec(EXPRARRAY_NEXT(exprs), level);
        }

        DBUG_VOID_RETURN;
    }
#endif

/**
 * Prints the given exprarray and all its sub-arrays to the DBUG output. Does nothing if
 * DBUG_OFF is defined.
 *
 * @param exprs: The exprarray to print
 */
void print_exprarray(node* exprs) {
    DBUG_ENTER("print_exprarray");
    DBUG_ASSERT((NODE_TYPE(exprs) == N_exprarray), "Wrong node type given!");

    // As the function only prints to the debug stream, we dont have to do anything
    // when DBUG_OFF is defined
    #ifndef DBUG_OFF
    print_exprarray_rec(exprs, 0);
    #else
    UNUSED_VAR(exprs);
    #endif

    DBUG_VOID_RETURN;
}

/**
 * Converts the given type to its string representation.
 * @param to_print: The type to convert
 * @return pointer to the string of the given type. String is
 *         located on the heap, so needs freeing.
 */
char* get_type_str(type to_print) {
    DBUG_ENTER("get_type_str");

    switch (to_print) {
        case TP_bool:
            DBUG_RETURN(STRcpy("BOOL"));
        case TP_bool_array:
            DBUG_RETURN(STRcpy("BOOL_ARRAY"));
        case TP_int:
            DBUG_RETURN(STRcpy("INT"));
        case TP_int_array:
            DBUG_RETURN(STRcpy("INT_ARRAY"));
        case TP_float:
            DBUG_RETURN(STRcpy("FLOAT"));
        case TP_float_array:
            DBUG_RETURN(STRcpy("FLOAT_ARRAY"));
        default:
            DBUG_RETURN(STRcpy("UNKNOWN"));
    }
}

char* get_nodetype_string(nodetype nt) {
    DBUG_ENTER("get_nodetype_string");

    switch(nt) {
        case N_undefined:
            DBUG_RETURN(STRcpy("undefined"));
        case N_program:
            DBUG_RETURN(STRcpy("program"));
        case N_declaration:
            DBUG_RETURN(STRcpy("declaration"));
        case N_function:
            DBUG_RETURN(STRcpy("function"));
        case N_basicparameter:
            DBUG_RETURN(STRcpy("basicparameter"));
        case N_arrayparameter:
            DBUG_RETURN(STRcpy("arrayparameter"));
        case N_arraydimensionvars:
            DBUG_RETURN(STRcpy("arraydimensionvars"));
        case N_functiondefinition:
            DBUG_RETURN(STRcpy("functiondefinition"));
        case N_functionbody:
            DBUG_RETURN(STRcpy("functionbody"));
        case N_functionvardeclaration:
            DBUG_RETURN(STRcpy("functionvardeclaration"));
        case N_localfunction:
            DBUG_RETURN(STRcpy("localfunction"));
        case N_statements:
            DBUG_RETURN(STRcpy("statements"));
        case N_basicvardeclaration:
            DBUG_RETURN(STRcpy("basicvardeclaration"));
        case N_arrayvardeclaration:
            DBUG_RETURN(STRcpy("arrayvardeclaration"));
        case N_exprarray:
            DBUG_RETURN(STRcpy("exprarray"));
        case N_globaldefinition:
            DBUG_RETURN(STRcpy("globaldefintion"));
        case N_for:
            DBUG_RETURN(STRcpy("for"));
        case N_while:
            DBUG_RETURN(STRcpy("while"));
        case N_dowhile:
            DBUG_RETURN(STRcpy("dowhile"));
        case N_global:
            DBUG_RETURN(STRcpy("global"));
        case N_error:
            DBUG_RETURN(STRcpy("error"));
        case N_funcall:
            DBUG_RETURN(STRcpy("funcall"));
        case N_funcallparameter:
            DBUG_RETURN(STRcpy("funcallparameter"));
        case N_return:
            DBUG_RETURN(STRcpy("return"));
        case N_branch:
            DBUG_RETURN(STRcpy("branch"));
        case N_expr:
            DBUG_RETURN(STRcpy("expr"));
        case N_monop:
            DBUG_RETURN(STRcpy("monop"));
        case N_constant:
            DBUG_RETURN(STRcpy("constant"));
        case N_floatconst:
            DBUG_RETURN(STRcpy("floatconst"));
        case N_intconst:
            DBUG_RETURN(STRcpy("intconst"));
        case N_boolconst:
            DBUG_RETURN(STRcpy("boolconst"));
        case N_cast:
            DBUG_RETURN(STRcpy("cast"));
        case N_varread:
            DBUG_RETURN(STRcpy("varread"));
        case N_varassign:
            DBUG_RETURN(STRcpy("varassign"));
        case N_arrayvarread:
            DBUG_RETURN(STRcpy("arrayvarread"));
        case N_arrayvarassign:
            DBUG_RETURN(STRcpy("arrayvarassign"));
        case N_binop:
            DBUG_RETURN(STRcpy("binop"));
        default:
            DBUG_ASSERT((FALSE), "Invalid nodetype given!");
            DBUG_RETURN(NULL);
    }
}

char* get_binop_str(binop bo_type) {
    DBUG_ENTER("get_binop_str");
    switch (bo_type) {
        case BO_add:
            DBUG_RETURN(STRcpy("+"));
        case BO_sub:
            DBUG_RETURN(STRcpy("-"));
        case BO_mul:
            DBUG_RETURN(STRcpy("*"));
        case BO_div:
            DBUG_RETURN(STRcpy("/"));
        case BO_mod:
            DBUG_RETURN(STRcpy("%"));
        case BO_lt:
            DBUG_RETURN(STRcpy("<"));
        case BO_le:
            DBUG_RETURN(STRcpy("<="));
        case BO_ge:
            DBUG_RETURN(STRcpy(">="));
        case BO_gt:
            DBUG_RETURN(STRcpy(">"));
        case BO_eq:
            DBUG_RETURN(STRcpy("=="));
        case BO_ne:
            DBUG_RETURN(STRcpy("!="));
        case BO_and:
            DBUG_RETURN(STRcpy("&&"));
        case BO_or:
            DBUG_RETURN(STRcpy("||"));
        default:
            DBUG_ASSERT((FALSE), "Incorrect binop type!");
            DBUG_RETURN(NULL);
        }
}

void copy_node_error_data(node* source, node* target) {
    DBUG_ENTER("copy_node_error_data");
    DBUG_ASSERT((source != NULL), "Source node was NULL!");
    DBUG_ASSERT((target != NULL), "Target node was NULL!");
    NODE_FILE(target) = NODE_FILE(source);
    NODE_LINE(target) = NODE_LINE(source);
    NODE_COL(target) = NODE_COL(source);

    DBUG_VOID_RETURN;
}

void size_t_to_str(char* buffer, size_t toconvert) {
    DBUG_ENTER("size_t_to_str");
    DBUG_ASSERT((buffer != NULL), "Buffer can't be NULL!");

    int result = snprintf(buffer, STR_BUF_SIZE, "%zu", toconvert);

    DBUG_ASSERT((result != STR_BUF_SIZE), "String buffer too small to hold toconvert");
    DBUG_ASSERT((result >= 0), "Error converting toconvert");

    DBUG_VOID_RETURN;
}
