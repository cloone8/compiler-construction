#include "boolcast.h"
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

node* CBCcast(node* arg_node, info* arg_info) {
    DBUG_ENTER("CBCcast");

    CAST_TARGET(arg_node) = TRAVdo(CAST_TARGET(arg_node), arg_info);
    type cast_to = CAST_CASTTO(arg_node);

    printf("%s", print_type(cast_to));
    printf("%s", print_type(TO_CAST(arg_info)));

    if (TO_CAST(arg_info) != TP_bool || cast_to != TP_bool) {
        if (cast_to == TP_bool) {
            /*casting towards bool*/
            
        } else {
            /*casting towards non-bool*/
            if (BOOL_VAL(arg_info)) {
                int toreturn = 1;
                printf("het is nu %d", toreturn);
            } else {
                int toreturn = 0;
                printf("het is nu %d", toreturn);
            }
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