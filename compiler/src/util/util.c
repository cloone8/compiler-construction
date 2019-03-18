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
    DBUG_PRINT("print_exprarray_rec", ("Entering exprarray (level %zu)", level));

    if(NODE_TYPE(EXPRARRAY_CURRENT(exprs)) == N_exprarray) {
        print_exprarray_rec(EXPRARRAY_CURRENT(exprs), level + 1);
    }

    DBUG_PRINT("print_exprarray_rec", ("Leaving exprarray (level %zu)", level));

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

    static char* result;

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

char* get_nodetype_string(nodetype nt) {
    DBUG_ENTER("get_nodetype_string");
    static char* toreturn = NULL;

    switch(nt) {
        case N_undefined:
            toreturn = "undefined";
            break;
        case N_program:
            toreturn = "program";
            break;
        case N_declaration:
            toreturn = "declaration";
            break;
        case N_function:
            toreturn = "function";
            break;
        case N_basicparameter:
            toreturn = "basicparameter";
            break;
        case N_arrayparameter:
            toreturn = "arrayparameter";
            break;
        case N_arraydimensionvars:
            toreturn = "arraydimensionvars";
            break;
        case N_functiondefinition:
            toreturn = "functiondefinition";
            break;
        case N_functionbody:
            toreturn = "functionbody";
            break;
        case N_functionvardeclaration:
            toreturn = "functionvardeclaration";
            break;
        case N_localfunction:
            toreturn = "localfunction";
            break;
        case N_statements:
            toreturn = "statements";
            break;
        case N_basicvardeclaration:
            toreturn = "basicvardeclaration";
            break;
        case N_arrayvardeclaration:
            toreturn = "arrayvardeclaration";
            break;
        case N_exprarray:
            toreturn = "exprarray";
            break;
        case N_globaldefinition:
            toreturn = "globaldefintion";
            break;
        case N_for:
            toreturn = "for";
            break;
        case N_while:
            toreturn = "while";
            break;
        case N_dowhile:
            toreturn = "dowhile";
            break;
        case N_global:
            toreturn = "global";
            break;
        case N_error:
            toreturn = "error";
            break;
        case N_funcall:
            toreturn = "funcall";
            break;
        case N_funcallparameter:
            toreturn = "funcallparameter";
            break;
        case N_return:
            toreturn = "return";
            break;
        case N_branch:
            toreturn = "branch";
            break;
        case N_expr:
            toreturn = "expr";
            break;
        case N_monop:
            toreturn = "monop";
            break;
        case N_constant:
            toreturn = "constant";
            break;
        case N_floatconst:
            toreturn = "floatconst";
            break;
        case N_intconst:
            toreturn = "intconst";
            break;
        case N_boolconst:
            toreturn = "boolconst";
            break;
        case N_cast:
            toreturn = "cast";
            break;
        case N_varread:
            toreturn = "varread";
            break;
        case N_varassign:
            toreturn = "varassign";
            break;
        case N_arrayvarread:
            toreturn = "arrayvarread";
            break;
        case N_arrayvarassign:
            toreturn = "arrayvarassign";
            break;
        case N_binop:
            toreturn = "binop";
            break;
        default:
            DBUG_ASSERT((FALSE), "Invalid nodetype given!");
    }
    DBUG_RETURN(toreturn);
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
