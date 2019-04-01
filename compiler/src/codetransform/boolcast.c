#include "boolcast.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free_node.h"
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
    type to_cast;
};

/*
 * INFO macros
 */
#define TO_CAST(__arg_info_struct__) ((__arg_info_struct__)->to_cast)
#define INT_VAL(__arg_info_struct__) ((__arg_info_struct__)->intval)
#define FLOAT_VAL(__arg_info_struct__) ((__arg_info_struct__)->floatval)
#define BOOL_VAL(__arg_info_struct__) ((__arg_info_struct__)->boolval)

static info* CBCmakeInfo(void) {
    DBUG_ENTER("CBCmakeInfo");

    info* result = (info*)MEMmalloc(sizeof(info));
    TO_CAST(result) = TP_unknown;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *CBCfreeInfo(info *info) {
    DBUG_ENTER("CBCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* CBCintconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCintconst");

    TO_CAST(arg_info) = TP_int;

    DBUG_RETURN(arg_node);
}

node* CBCboolconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCboolconst");

    TO_CAST(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}

node* CBCfloatconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCfloatconst");

    TO_CAST(arg_info) = TP_float;

    DBUG_RETURN(arg_node);
}

node* CBCvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCvarread");

    symbol_table_entry *entry = VARREAD_VARPOINTER(arg_node);
    TO_CAST(arg_info) = entry->vartype;

    DBUG_RETURN(arg_node);
}

node* CBCbinop(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCbinop");

    symbol_table_entry *entry = VARREAD_VARPOINTER(arg_node);
    TO_CAST(arg_info) = entry->vartype;

    info* left_info = CBCmakeInfo();

    /* We need to  store types of the left and right operands.*/
    TRAVdo(BINOP_LEFT(arg_node), left_info);

    switch(BINOP_BINOPERATORS(arg_node)) {
        case BO_lt:
        case BO_le:
        case BO_eq:
        case BO_ne:
        case BO_ge:
        case BO_gt:
            TO_CAST(arg_info) = TP_bool;
            break;
        default:
            TO_CAST(arg_info) = TO_CAST(left_info);
            break;
    }

    CBCfreeInfo(left_info);

    DBUG_RETURN(arg_node);
}

/* Handle the conversion from an int or float towards a bool. */
node* handle_boolcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("handle_boolcast");

    if(TO_CAST(arg_info) == TP_float) {
        /*We have a float.*/

        node* condition = TBmakeBinop(BO_eq, CAST_TARGET(arg_node), TBmakeExpr(TBmakeConstant(TBmakeFloatconst(0.0))));
        node* toreturn = TBmakeConditionalexpr(TBmakeExpr(condition),
                            TBmakeExpr(TBmakeConstant(TBmakeBoolconst(FALSE))),
                            TBmakeExpr(TBmakeConstant(TBmakeBoolconst(TRUE))));
        DBUG_RETURN(toreturn);
    } else {
        /*We have an int.*/

        node* condition = TBmakeBinop(BO_eq, CAST_TARGET(arg_node), TBmakeExpr(TBmakeConstant(TBmakeIntconst(0))));
        node* toreturn = TBmakeConditionalexpr(TBmakeExpr(condition),
                            TBmakeExpr(TBmakeConstant(TBmakeBoolconst(FALSE))),
                            TBmakeExpr(TBmakeConstant(TBmakeBoolconst(TRUE))));
        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

/* Handle the conversion from a bool towards an int or towards a float. */
node* handle_from_boolcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("handle_from_boolcast");

    if(CAST_CASTTO(arg_node) == TP_float) {
        /*We have a float.*/

        node* condition = TBmakeBinop(BO_eq, CAST_TARGET(arg_node), TBmakeExpr(TBmakeConstant(TBmakeBoolconst(TRUE))));
        node* toreturn = TBmakeConditionalexpr(TBmakeExpr(condition),
                            TBmakeExpr(TBmakeConstant(TBmakeFloatconst(1.0))),
                            TBmakeExpr(TBmakeConstant(TBmakeFloatconst(0.0))));
        DBUG_RETURN(toreturn);
    } else {
        /*We have an int.*/

        node* condition = TBmakeBinop(BO_eq, CAST_TARGET(arg_node), TBmakeExpr(TBmakeConstant(TBmakeBoolconst(TRUE))));
        node* toreturn = TBmakeConditionalexpr(TBmakeExpr(condition),
                            TBmakeExpr(TBmakeConstant(TBmakeIntconst(1))),
                            TBmakeExpr(TBmakeConstant(TBmakeIntconst(0))));
        DBUG_RETURN(toreturn);
    }

    DBUG_RETURN(arg_node);
}

node* CBCcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCcast");

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);

    if (TO_CAST(arg_info) == TP_bool || CAST_CASTTO(arg_node) == TP_bool) {
        if (CAST_CASTTO(arg_node) == TP_bool) {
            /*casting towards bool*/
            node* old_cast = arg_node;
            arg_node = handle_boolcast(arg_node, arg_info);
            TRAVpush(TR_free);
            CAST_TARGET(old_cast) = NULL;
            FREEcast(old_cast, NULL);
            TRAVpop();
        } else {
            /*casting towards non-bool*/
            node *old_cast = arg_node;
            arg_node = handle_from_boolcast(arg_node, arg_info);
            TRAVpush(TR_free);
            CAST_TARGET(old_cast) = NULL;
            FREEcast(old_cast, NULL);
            TRAVpop();
        }
        DBUG_RETURN(arg_node);
    }

    DBUG_RETURN(arg_node);

}

node *CBCdo(node *syntaxtree) {
    DBUG_ENTER("CBCdo");

    TRAVpush(TR_cbc);
    info *arg_info = CBCmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    CBCfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
