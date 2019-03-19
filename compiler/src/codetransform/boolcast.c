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
    int intval;
    float floatval;
    bool boolval;
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
    INT_VAL(result) = 0;
    FLOAT_VAL(result) = 0;
    BOOL_VAL(result) = FALSE;

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

    INT_VAL(arg_info) = INTCONST_VALUE(arg_node);
    TO_CAST(arg_info) = TP_int;

    DBUG_RETURN(arg_node);
}

node* CBCboolconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCboolconst");

    BOOL_VAL(arg_info) = BOOLCONST_VALUE(arg_node);
    TO_CAST(arg_info) = TP_bool;

    DBUG_RETURN(arg_node);
}

node* CBCfloatconst(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCfloatconst");

    FLOAT_VAL(arg_info) = FLOATCONST_VALUE(arg_node);
    TO_CAST(arg_info) = TP_float;

    DBUG_RETURN(arg_node);
}

node* CBCvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCvarread");

    symbol_table_entry *entry = VARREAD_VARPOINTER(arg_node);
    TO_CAST(arg_info) = entry->vartype;

    DBUG_RETURN(arg_node);
}

/* Handle the conversion from an int or float towards a bool. */
node* handle_boolcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("handle_boolcast");

    if(TO_CAST(arg_info) == TP_float) {
        /*We have a float.*/
        if (FLOAT_VAL(arg_info) != 0) {
            DBUG_RETURN(TBmakeBoolconst(TRUE));
        } else {
            DBUG_RETURN(TBmakeBoolconst(FALSE));
        }
    }
    else {
        /*We have an int.*/
        if (INT_VAL(arg_info) != 0) {
            DBUG_RETURN(TBmakeBoolconst(TRUE));
        } else {
            DBUG_RETURN(TBmakeBoolconst(FALSE));
        }
    }


    DBUG_RETURN(arg_node);
}

/* Handle the conversion from a bool towards an int or towards a float. */
node* handle_from_boolcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("handle_from_boolcast");

    if (CAST_CASTTO(arg_node) == TP_float) {
        /*We need a float cast.*/
        if (BOOL_VAL(arg_info) == TRUE) {
            arg_node = TBmakeFloatconst(1.0);
        } else {
            arg_node = TBmakeFloatconst(0.0);
        }
    } else {
        /*We need an int cast*/
        if (BOOL_VAL(arg_info) == TRUE) {
            arg_node = TBmakeIntconst(1);
        } else {
            arg_node = TBmakeIntconst(0);
        }
    }

    DBUG_RETURN(arg_node);
}

node* CBCcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCcast");

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);

    if (TO_CAST(arg_info) != TP_bool || CAST_CASTTO(arg_node) != TP_bool) {
        if (CAST_CASTTO(arg_node) == TP_bool) {
            /*casting towards bool*/
            node* old_cast = arg_node;
            arg_node = TBmakeConstant(handle_boolcast(arg_node, arg_info));
            TRAVpush(TR_free);
            FREEcast(old_cast, NULL);
            TRAVpop();
            // CBCfreeInfo(arg_info);
        } else {
            /*casting towards non-bool*/
            node *old_cast = arg_node;
            arg_node = TBmakeConstant(handle_from_boolcast(arg_node, arg_info));
            TRAVpush(TR_free);
            FREEcast(old_cast, NULL);
            TRAVpop();
            // CBCfreeInfo(arg_info);
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
