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
    fun_symbol_table* currentfuntable;
    fun_symbol_table* currentsymtable;
    size_t paramcount;
    type* paramtypes;
    node* newparamnodes;
};

/*
 * INFO macros
 */

#define CURRENT_FUNCTION_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentfuntable)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define PARAMCOUNT(__arg_info_struct__) ((__arg_info_struct__)->paramcount)
#define PARAMTYPES(__arg_info_struct__) ((__arg_info_struct__)->paramtypes)
#define NEWNODES(__arg_info_struct__) ((__arg_info_struct__)->newparamnodes)

/**
 * Creates a new info struct and sets the initial values
 */
static info *APPmakeInfo(void) {
    DBUG_ENTER("APPmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));

    result->currentfuntable = NULL;
    result->currentsymtable = NULL;
    result->paramcount = 0;
    result->paramtypes = NULL;
    result->newparamnodes = NULL;

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

    printf("%s\n", FUNCTION_NAME(arg_node));

    // Go through parameters and make them update themselves
    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    // Now recheck the parameters and update the symbol table entry for this function

    fun_symbol_table_entry* entry = FUNCTION_FUNPOINTER(arg_node);
    MEMfree(entry->paramtypes);
    entry->paramamount = PARAMCOUNT(arg_info);
    entry->paramtypes = PARAMTYPES(arg_info);

    PARAMCOUNT(arg_info) = 0;
    PARAMTYPES(arg_info) = NULL;

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* APParrayparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("APParrayparameter");
    printf("%s\n", ARRAYPARAMETER_NAME(arg_node));
    node* toreturn = arg_node;

    size_t myindex = PARAMCOUNT(arg_info)++;

    info* dims_info = APPmakeInfo();

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), dims_info);

    size_t dimcount = PARAMCOUNT(dims_info);

    if(dimcount != 0) {
        toreturn = NEWNODES(dims_info);
        append_functionparameter_to_tail(NEWNODES(dims_info), arg_node);
    }

    myindex += dimcount;

    APPfreeInfo(dims_info);

    PARAMCOUNT(arg_info) += dimcount;

    // Go to the next parameter
    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    } else {
        PARAMTYPES(arg_info) = MEMmalloc(PARAMCOUNT(arg_info) * sizeof(type));
    }

    PARAMTYPES(arg_info)[myindex] = ARRAYPARAMETER_TYPE(arg_node);

    if(dimcount != 0) {
        for(size_t i = myindex - 1; i > (myindex - dimcount); i--) {
            printf("%zu %zu\n", i, myindex-dimcount);
            printf("Looping...\n");
            PARAMTYPES(arg_info)[myindex] = TP_int;
        }
    }

    DBUG_RETURN(toreturn);
}

node* APParraydimensionvars(node* arg_node, info* arg_info) {
    DBUG_ENTER("APParraydimensionvars");

    PARAMCOUNT(arg_info)++;
    node* new_param = TBmakeBasicparameter(STRcpy(ARRAYDIMENSIONVARS_NAME(arg_node)), TP_int, NULL);
    BASICPARAMETER_VARPOINTER(new_param) = ARRAYDIMENSIONVARS_VARPOINTER(arg_node);

    if(NEWNODES(arg_info) == NULL) {
        NEWNODES(arg_info) = new_param;
    } else {
        append_functionparameter_to_tail(NEWNODES(arg_info), new_param);
    }

    // Go to the next parameter
    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL) {
        ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYDIMENSIONVARS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* APPfunctiondefinition(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfunctiondefinition");

    fun_symbol_table* old_table = CURRENT_FUNCTION_TABLE(arg_info);
    CURRENT_FUNCTION_TABLE(arg_info) = FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_FUNCTION_TABLE(arg_info) = old_table;

    DBUG_RETURN(arg_node);
}

node* APPfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfuncall");

    DBUG_RETURN(arg_node);
}

node* APPdo(node* syntaxtree) {
    DBUG_ENTER("APPdo");

    info* arg_info = APPmakeInfo();
    CURRENT_FUNCTION_TABLE(arg_info) = PROGRAM_FUNSYMBOLTABLE(syntaxtree);

    TRAVpush(TR_app);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = APPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
