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
    binop to_evaluate;
    type pred;
};

/*
 * INFO macros
 */
#define CURR_BINOP(__arg_info_struct__) ((__arg_info_struct__)->to_evaluate)
#define CURR_BOOL(__arg_info_struct__) ((__arg_info_struct__)->pred)

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
    CURR_BINOP(result) = BO_unknown;
    CURR_BOOL(result) = TP_unknown;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *BDCfreeInfo(info *info) {
    DBUG_ENTER("BDCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* BDCboolconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("BDCboolconst");

    CURR_BOOL(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}

node* BDCvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("BDCvarread");

    symbol_table_entry* entry = VARREAD_VARPOINTER(arg_node);
    CURR_BOOL(arg_info) = entry->vartype;

    DBUG_RETURN(arg_node);
}

node* BDCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("BDCbinop");

    node* cond_expr = NULL;

    CURR_BINOP(arg_info) = BINOP_BINOPERATORS(arg_node);

    info* left = BDCmakeInfo();
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), left);

    info *right = BDCmakeInfo();
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), right);

    if (CURR_BINOP(arg_info) == BO_and) {
        if (CURR_BOOL(left) == TP_bool && CURR_BOOL(right) == TP_bool) {
            cond_expr = TBmakeConditionalexpr(BINOP_LEFT(arg_node), BINOP_RIGHT(arg_node), TBmakeExpr(TBmakeConstant((TBmakeBoolconst(FALSE)))));
        }
    }

    if (CURR_BINOP(arg_info) == BO_or) {
        if (CURR_BOOL(left) == TP_bool && CURR_BOOL(right) == TP_bool) {
            cond_expr = TBmakeConditionalexpr(BINOP_LEFT(arg_node), TBmakeExpr(TBmakeConstant((TBmakeBoolconst(TRUE)))), BINOP_RIGHT(arg_node));
        }
    }

    BDCfreeInfo(left);
    BDCfreeInfo(right);

    if (cond_expr != NULL) {
        printf("made condexpr");
        DBUG_RETURN(cond_expr);
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
