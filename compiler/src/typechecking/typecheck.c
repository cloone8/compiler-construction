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

node* TCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("TCbinop");

    DBUG_RETURN(arg_node);
}

node *TCdo(node *syntaxtree) {
    DBUG_ENTER("TCdo");

    TRAVpush(TR_tc);
    info *arg_info = TCmakeInfo();


    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}