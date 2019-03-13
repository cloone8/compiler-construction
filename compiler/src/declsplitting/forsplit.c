#include "forsplit.h"
#include "symbol_table.h"

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
    symbol_table* current_table;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *FSPmakeInfo(void) {
    DBUG_ENTER("FSPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FSPfreeInfo(info *info) {
    DBUG_ENTER("FSPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* FSPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfunctiondefinition");

    DBUG_RETURN(arg_node);
}

node* FSPfunctionbody(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfunctionbody");

    DBUG_RETURN(arg_node);
}

node* FSPfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FSPfor");

    DBUG_RETURN(arg_node);
}

node *FSPdo(node* syntaxtree) {
    DBUG_ENTER("FSPdo");

    info* arg_info = FSPmakeInfo();

    TRAVpush(TR_fsp);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = FSPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
