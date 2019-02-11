/*****************************************************************************
 *
 * Module: count_ops
 *
 * Prefix: CO
 *
 * Description:
 *
 * This module inplements a compiler phase which counts all the present operators,
 * and adds them to a new root node. This root node is then inserted at the start
 * of the syntax tree.
 *
 *****************************************************************************/

#include "count_ops.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"

/*
 * INFO structure
 */
struct INFO
{
    int add_count;
    int sub_count;
    int mul_count;
    int div_count;
    int mod_count;
    node *module_node;
};

/*
 * INFO macros
 */
#define FOUND_ADD(info_struct) (((info_struct)->add_count)++)
#define FOUND_SUB(info_struct) (((info_struct)->sub_count)++)
#define FOUND_MUL(info_struct) (((info_struct)->mul_count)++)
#define FOUND_DIV(info_struct) (((info_struct)->div_count)++)
#define FOUND_MOD(info_struct) (((info_struct)->mod_count)++)
#define SET_MODULE_NODE(info_struct, new_module_node) (((info_struct)->module_node) = (new_module_node))

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *COmakeInfo(void)
{
    info *result;

    DBUG_ENTER("COmakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    result->add_count = 0;
    result->sub_count = 0;
    result->mul_count = 0;
    result->div_count = 0;
    result->mod_count = 0;
    result->module_node = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *COfreeInfo(info *info)
{
    DBUG_ENTER("COfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/*
 * Traversal functions
 */

/**
 * Handles the traversal of a single BINOP node. Checks the type of operator and
 * adds that to the total count
 */
node *CObinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("COcount");

    // Continue down the tree recursively
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    switch (BINOP_OP(arg_node))
    {
    case BO_add:
        FOUND_ADD(arg_info);
        break;
    case BO_sub:
        FOUND_SUB(arg_info);
        break;
    case BO_mul:
        FOUND_MUL(arg_info);
        break;
    case BO_div:
        FOUND_DIV(arg_info);
        break;
    case BO_mod:
        FOUND_MOD(arg_info);
        break;
    default:
        break;
    }

    DBUG_RETURN(arg_node);
}

/**
 * Traversal start function. Starts creates a new root node, and starts
 * the traversal through the syntax tree. Finally sets the attributes of the
 * new root node to their correct values.
 */
node *COdoCount(node *syntaxtree)
{
    info *arg_info;

    DBUG_ENTER("COdoCount");

    arg_info = COmakeInfo();
    node *module_node = TBmakeModule(0, 0, 0, 0, 0, syntaxtree);

    TRAVpush(TR_co);
    module_node = TRAVdo(module_node, arg_info);
    TRAVpop();

    MODULE_ADD_COUNT(module_node) = arg_info->add_count;
    MODULE_SUB_COUNT(module_node) = arg_info->sub_count;
    MODULE_MUL_COUNT(module_node) = arg_info->mul_count;
    MODULE_DIV_COUNT(module_node) = arg_info->div_count;
    MODULE_MOD_COUNT(module_node) = arg_info->mod_count;

    arg_info = COfreeInfo(arg_info);

    DBUG_RETURN(module_node);
}
