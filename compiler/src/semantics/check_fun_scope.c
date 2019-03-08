#include "check_fun_scope.h"
#include "fun_symbol_table.h"

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
struct INFO
{
    fun_symbol_table *current_table;
    size_t paramcount;
    type *currentparams;
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
static info *CFSmakeInfo(void)
{
    DBUG_ENTER("CFSmakeInfo");

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
static info *CFSfreeInfo(info *info)
{
    DBUG_ENTER("CFSfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *CFSfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuctiiondefinition");

    DBUG_RETURN(NULL);
}

node *CFSfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("CFSfuncall");

    DBUG_RETURN(NULL);
}

node *CFSdo(node *syntaxtree) {
    DBUG_ENTER("CFSdo");

    // TRAVpush(TR_cfs);
    // info *arg_info = STBmakeInfo();

    // symbol_table *global_var_table = symbol_table_make(NULL);
    // PROGRAM_VARSYMBOLTABLE(syntaxtree) = global_var_table;
    // CURRENT_SYMBOL_TABLE(arg_info) = global_var_table;

    // syntaxtree = TRAVdo(syntaxtree, arg_info);

    // STBfreeInfo(arg_info);
    // TRAVpop();

    DBUG_RETURN(syntaxtree);
}
