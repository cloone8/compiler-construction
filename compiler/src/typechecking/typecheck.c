#include "typecheck.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    fun_symbol_table *current_fun_symbol_table;
    symbol_table *current_symbol_table;
};


/*
 * INFO macros
 */

#define CURRENT_FUN_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_fun_symbol_table)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_symbol_table)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *TCmakeInfo(void) {
    DBUG_ENTER("TCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_FUN_SYMBOL_TABLE(result) = NULL;
    CURRENT_SYMBOL_TABLE(result) = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *TCfreeInfo(info *info) {
    DBUG_ENTER("TCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* TCvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCvarassign");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    char* varname = VARASSIGN_VARNAME(arg_node);
    symbol_table_entry* entry = symbol_table_get_name(current, varname);

    type needed = entry->vartype;
    type given;

    node* expr = VARASSIGN_VALUE(arg_node);
    node* constnt = EXPR_CURRENT(expr);
    node* type_tocheck = CONSTANT_CURRENT(constnt);

    switch (NODE_TYPE(type_tocheck)) {
        case N_intconst:
            given = TP_int;
            break;
        case N_boolconst:
            given = TP_bool;
            break;
        case N_floatconst:
            given = TP_float;
            break;
        default:
            given = TP_unknown;
            break;
    }

    if (given != needed) {
        CTIerror("Varassign has wrong types, %s needs type %s but got %s", varname, print_type(needed), print_type(given));
    }

    DBUG_RETURN(arg_node);
}

node* TCfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCbinop");

    char* name = FUNCALL_FUNCTIONNAME(arg_node);
    node* params = FUNCALL_PARAMS(arg_node);

    size_t paramamount = fun_symbol_table_get_param_amount(CURRENT_FUN_SYMBOL_TABLE(arg_info), name);

    type given[paramamount];
    type* paramtypes = fun_symbol_table_get_param_types(CURRENT_FUN_SYMBOL_TABLE(arg_info), name);

    node* expr;
    node* constnt;
    node* type_tocheck;

    /* We only need to check the types if we have parameters. */
    if(paramamount != 0) {
        /* Wrap nodes to types and set the given array types. */
        for (size_t i = 0; i < paramamount; i++) {
            expr = FUNCALLPARAMETER_VALUE(params);
            constnt = EXPR_CURRENT(expr);
            type_tocheck = CONSTANT_CURRENT(constnt);

            switch (NODE_TYPE(type_tocheck)) {
                case N_intconst:
                    given[i] = TP_int;
                    break;
                case N_boolconst:
                    given[i] = TP_bool;
                    break;
                case N_floatconst:
                    given[i] = TP_float;
                    break;
                default:
                    break;
            }
            params = FUNCALLPARAMETER_NEXT(params);
        }
    }

    /*Give error message when type arrays mismatch.*/
    for (size_t i = 0; i < paramamount; i++) {
        if (given[i] != paramtypes[i]) {
            CTIerror("Wrong types for function %s, argument %d expected type %s, but got type %s", name, i, print_type(paramtypes[i]), print_type(given[i]));
        }
    }

    DBUG_RETURN(arg_node);
}

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();


    symbol_table* global_symbol_table = PROGRAM_VARSYMBOLTABLE(syntaxtree);
    CURRENT_SYMBOL_TABLE(arg_info) = global_symbol_table;

    fun_symbol_table *global_fun_table = PROGRAM_FUNSYMBOLTABLE(syntaxtree);
    CURRENT_FUN_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}