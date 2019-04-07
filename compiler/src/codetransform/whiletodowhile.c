#include "typecheck.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "free_node.h"
#include "copy_node.h"
#include "copy_info.h"
#include "lookup_table.h"
#include <string.h>
#include "copy.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

/**
 * This function trasnforms a while-loop to a do-while-loop. To make sure the conditon from the original while is checked the first time,
 * we wrap the new do-while-loop inside an if-statement.
 **/
node* WTDWwhile(node* arg_node, info* arg_info) {
    DBUG_ENTER("WTDWwhile");
    UNUSED_VAR(arg_info);

    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), NULL);
    WHILE_CODE(arg_node) = TRAVdo(WHILE_CODE(arg_node), NULL);

    node* code = WHILE_CODE(arg_node);
    node* cond = WHILE_CONDITION(arg_node);

    /* Create shallow copies of the node so we don't free twice. */
    node *retval = TBmakeDowhile(COPYdoCopy(cond), COPYdoCopy(code));
    retval = TBmakeBranch(COPYdoCopy(cond), TBmakeStatements(retval, NULL), NULL);

    /* Free the detached while-loop node. */
    TRAVpush(TR_free);
    FREEwhile(arg_node, arg_info);
    TRAVpop();

    DBUG_RETURN(retval);
}

node *WTDWdo(node *syntaxtree) {
    DBUG_ENTER("WTWDdo");

    TRAVpush(TR_wtdw);

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
