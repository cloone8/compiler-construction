#include "boolcomp.h"
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
    bool predicate;
};

/*
 * INFO macros
 */
#define PREDICATE(__arg_info_struct__) ((__arg_info_struct__)->predicate)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *BDCmakeInfo(void)
{
    DBUG_ENTER("BDCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    PREDICATE(result) = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *BDCfreeInfo(info *info)
{
    DBUG_ENTER("BDCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* BDCboolconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("BDCboolconst");

    printf("booll");

    PREDICATE(arg_info) = BOOLCONST_VALUE(arg_node);

    DBUG_RETURN(arg_node);
}

node* handleAnd(node* arg_node, info* arg_info) {
    DBUG_ENTER("handleAnd");

    info* val = BDCmakeInfo();

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), val);

    if (PREDICATE(val)) {
        printf("ture");
    }

    BDCfreeInfo(val);

    DBUG_RETURN(arg_node);
}

node* handleOr(node* arg_node, info* arg_info) {
    DBUG_ENTER("handleOr");

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node* BDCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("BDCbinop");

    binop current = BINOP_BINOPERATORS(arg_node);
    switch (current)
    {
        case BO_and:
            arg_node = handleAnd(arg_node, arg_info);
            break;
        case BO_or:
            arg_node = handleOr(arg_node, arg_info);
            break;
        default:
            break;
    }

    DBUG_RETURN(arg_node);
}

node *BDCdo(node *syntaxtree) {
    DBUG_ENTER("BDCdo");

    TRAVpush(TR_bdc);
    info *arg_info = BDCmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    BDCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
