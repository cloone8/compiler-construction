#include "array_varsplit.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

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
static info *BSPmakeInfo(void) {
    DBUG_ENTER("BSPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *BSPfreeInfo(info *info) {
    DBUG_ENTER("BSPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *ASPdo(node *syntaxtree) {
    return NULL;
}
