#include "arrayparams.h"

#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "fun_symbol_table.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "ctinfo.h"

#include "util.h"

/*
 * INFO structure
 */
struct INFO {
    fun_symbol_table* currentsymtable;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)

/**
 * Creates a new info struct and sets the initial values
 */
static info *APPmakeInfo(void) {
    DBUG_ENTER("APPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    result->currentsymtable = NULL;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *APPfreeInfo(info *info) {
    DBUG_ENTER("APPfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node* APPfunction(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfunction");

    // Go through parameters and make them update themselves
    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    // Now recheck the parameters and update the symbol table entry for this function

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* APParrayparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("APParrayparameter");

    // // Go through parameters and make them update themselves
    // if(FUNCTION_PARAMETERS(arg_node) != NULL) {
    //     FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    // }

    // // Now recheck the parameters and update the symbol table entry for this function

    // if(FUNCTION_DEFINITION(arg_node) != NULL) {
    //     FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    // }

    DBUG_RETURN(arg_node);
}

node* APPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfunctiondefinition");

    fun_symbol_table* old_table = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* APPfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfuncall");

    DBUG_RETURN(arg_node);
}

node* APPdo(node* syntaxtree) {
    DBUG_ENTER("APPdo");

    info* arg_info = APPmakeInfo();
    CURRENT_SYMBOL_TABLE(arg_info) = PROGRAM_FUNSYMBOLTABLE(syntaxtree);

    TRAVpush(TR_app);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = APPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
