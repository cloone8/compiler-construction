#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

#include "build_function_table.h"

#include "fun_symbol_table.h"

/*
 * INFO structure
 */
struct INFO {
    fun_symbol_table* current_table;
    size_t paramcount;
    type* currentparams;
    size_t current_scope;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define CURRENT_SCOPE(__arg_info_struct__) ((__arg_info_struct__)->current_scope)
#define PARAMS(__arg_info_struct__) ((__arg_info_struct__)->currentparams)
#define PARAMCOUNT(__arg_info_struct__) ((__arg_info_struct__)->paramcount)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *FTBmakeInfo(void) {
    DBUG_ENTER("FTBmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;
    CURRENT_SCOPE(result) = 0;
    PARAMS(result) = NULL;
    PARAMCOUNT(result) = 0;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FTBfreeInfo(info *info) {
    DBUG_ENTER("FTBfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *FTBfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBfunction");

    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    type* parameters = PARAMS(arg_info);
    size_t paramcount = PARAMCOUNT(arg_info);

    PARAMS(arg_info) = NULL;
    PARAMCOUNT(arg_info) = 0;

    // fun_symbol_table_add(CURRENT_SYMBOL_TABLE(arg_node))

    DBUG_RETURN(arg_node);
}

node *FTBfunctionbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBfunctionbody");

    DBUG_RETURN(arg_node);
}

node *FTBbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBbasicparameter");

    DBUG_RETURN(arg_node);
}

node *FTBarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBarrayparameter");

    DBUG_RETURN(arg_node);
}

node *FTBdo(node *syntaxtree) {
    DBUG_ENTER("FTBdo");

    TRAVpush(TR_ftb);
    info* arg_info = FTBmakeInfo();

    fun_symbol_table* global_fun_table = fun_symbol_table_make(NULL);
    PROGRAM_FUNSYMBOLTABLE(syntaxtree) = global_fun_table;
    CURRENT_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    FTBfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
