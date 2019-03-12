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

node* TCfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCbinop");

    char* name = FUNCALL_FUNCTIONNAME(arg_node);
    node* params = FUNCALL_PARAMS(arg_node);

    size_t paramamount = fun_symbol_table_get_param_amount(CURRENT_FUN_SYMBOL_TABLE(arg_info), name);

    type given[paramamount];

    if(paramamount != 0) {

        for (size_t i = 0; i < paramamount; i++) {
            node* expr = FUNCALLPARAMETER_VALUE(params);
            node* constnt = EXPR_CURRENT(expr);
            node* type_c = CONSTANT_CURRENT(constnt);

            switch (NODE_TYPE(type_c)) {
                case N_intconst:
                    printf("bool");
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
        }
    }

    type* paramtypes = fun_symbol_table_get_param_types(CURRENT_FUN_SYMBOL_TABLE(arg_info), name);

    for (size_t i = 0; i < paramamount; i++) {
        if (given[i] != paramtypes[i]) {
            CTIerror("wrong types");
        }
    }

        DBUG_RETURN(arg_node);
}

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();


    fun_symbol_table *global_fun_table = PROGRAM_FUNSYMBOLTABLE(syntaxtree);
    CURRENT_FUN_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}