#include "fortowhile.h"
#include "types.h"
#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "free_node.h"
#include "traverse.h"
#include "memory.h"

#include "str.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {

};

/*
 * INFO macros
 */

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *FTWmakeInfo(void) {
    DBUG_ENTER("FTWmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *FTWfreeInfo(info *info) {
    DBUG_ENTER("FTWfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* FTWfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("FTWfor");

    DBUG_RETURN(arg_node);
}

node* FTWdo(node* syntaxtree) {
    DBUG_ENTER("FTWdo");

    TRAVpush(TR_ftw);
    info *arg_info = FTWmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    FTWfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
