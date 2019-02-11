/*****************************************************************************
 *
 * Module: count_ids
 *
 * Prefix: CI
 *
 * Description:
 *
 *  This module counts the total number of ID mentions in the syntax tree, and
 *  prints them during compilation
 *****************************************************************************/

#include "count_ops.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "count_map.h"
#include "str.h"
#include "ctinfo.h"

/*
 * INFO structure
 */
struct INFO
{
    countMap* map;
};

/*
 * INFO functions
 */

/**
 * Creates a new info struct
 */
static info *CImakeInfo(void)
{
    info *result;

    DBUG_ENTER("CImakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    result->map = CMcreate();

    DBUG_RETURN(result);
}

/**
 * Frees the given info struct from memory
 */
static info *CIfreeInfo(info *info)
{
    DBUG_ENTER("COfreeInfo");

    CMfree(info->map);
    info = MEMfree(info);

    DBUG_RETURN(info);
}

/**
 * Adds another occurence of the given variable name to the variable map
 */
static void CIaddVarOccurence(info *arg_info, char* varname) {
    CMaddOccurence(arg_info->map, varname);
}

/*
 * Traversal functions
 */

/**
 * Handles the traversal of a single VAR node. Gets the name
 * of the given variable and adds another occurence of it.
 */
node *CIvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CIvar");

    char* varname = VAR_NAME(arg_node);

    CIaddVarOccurence(arg_info, varname);

    DBUG_RETURN(arg_node);
}

/**
 * Handles the traversal of a single VARLET node. Gets the name
 * of the given variable and adds another occurence of it.
 */
node *CIvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CIvarlet");

    char* varname = VARLET_NAME(arg_node);

    CIaddVarOccurence(arg_info, varname);

    DBUG_RETURN(arg_node);
}

/**
 * Traversal start function. Starts the traversal through the syntax tree,
 * and at the end counts the total number of variables and prints them during
 * compilation
 */
node *CIdoCount(node *syntaxtree)
{
    info *arg_info;

    DBUG_ENTER("CIdoCount");

    arg_info = CImakeInfo();

    TRAVpush(TR_ci);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    countMap* varmap = arg_info->map;

    for(int i = 0; i < CMgetItemCount(varmap); i++) {
        countMapItem* item = varmap->items[i];
        CTInote("Count of ID %s: %d\n", item->name, item->count);
    }

    arg_info = CIfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
