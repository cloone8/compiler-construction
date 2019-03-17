#include "arrayparams.h"

#include "types.h"
#include "dbug.h"
#include "memory.h"
#include "fun_symbol_table.h"
#include "symbol_table.h"
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
    symbol_table* currentsymtable;
    size_t paramcount;
    type* paramtypes;
    node* newparamnodes;
    bool infuncall;
};

/*
 * INFO macros
 */

#define CURRENT_FUNCTION_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentfuntable)
#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->currentsymtable)
#define PARAMCOUNT(__arg_info_struct__) ((__arg_info_struct__)->paramcount)
#define PARAMTYPES(__arg_info_struct__) ((__arg_info_struct__)->paramtypes)
#define NEWNODES(__arg_info_struct__) ((__arg_info_struct__)->newparamnodes)
#define IN_FUNCALL(__arg_info_struct__) ((__arg_info_struct__)->infuncall)

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
    result->infuncall = FALSE;

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

    size_t prev_paramcount = PARAMCOUNT(arg_info);
    type* prev_types = PARAMTYPES(arg_info);

    PARAMCOUNT(arg_info) = 0;
    PARAMTYPES(arg_info) = NULL;

    // Go through parameters and make them update themselves
    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    // Now recheck the parameters and update the symbol table entry for this function

    fun_symbol_table_entry* entry = FUNCTION_FUNPOINTER(arg_node);
    MEMfree(entry->paramtypes);
    entry->paramamount = PARAMCOUNT(arg_info);
    entry->paramtypes = PARAMTYPES(arg_info);

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    PARAMCOUNT(arg_info) = prev_paramcount;
    PARAMTYPES(arg_info) = prev_types;

    DBUG_RETURN(arg_node);
}

node* APParrayparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("APParrayparameter");
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

    printf("Total dims: %zu\n", dimcount);
    printf("Setting param %s index %zu\n to %s\n", ARRAYPARAMETER_NAME(arg_node), myindex, print_type(ARRAYPARAMETER_TYPE(arg_node)));
    PARAMTYPES(arg_info)[myindex] = ARRAYPARAMETER_TYPE(arg_node);

    if(dimcount != 0) {

        for(size_t i = myindex - dimcount; i < myindex; i++) {
            printf("Setting param %s dimension index %zu\n to %s\n", ARRAYPARAMETER_NAME(arg_node), i, print_type(TP_int));
            PARAMTYPES(arg_info)[i] = TP_int;
        }
    }

    DBUG_RETURN(toreturn);
}

node* APPbasicparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPbasicparameter");

    PARAMCOUNT(arg_info)++;

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    } else {
        PARAMTYPES(arg_info) = MEMmalloc(PARAMCOUNT(arg_info) * sizeof(type));
    }
    printf("Setting param %s index %zu\n to %s\n", BASICPARAMETER_NAME(arg_node), PARAMCOUNT(arg_info) - 1, print_type(BASICPARAMETER_TYPE(arg_node)));
    PARAMTYPES(arg_info)[PARAMCOUNT(arg_info) - 1] = BASICPARAMETER_TYPE(arg_node);

    DBUG_RETURN(arg_node);
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

    fun_symbol_table* old_fun_table = CURRENT_FUNCTION_TABLE(arg_info);
    symbol_table* old_var_table = CURRENT_SYMBOL_TABLE(arg_info);

    CURRENT_FUNCTION_TABLE(arg_info) = FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node);
    CURRENT_SYMBOL_TABLE(arg_info) = FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node);

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_FUNCTION_TABLE(arg_info) = old_fun_table;
    CURRENT_SYMBOL_TABLE(arg_info) = old_var_table;

    DBUG_RETURN(arg_node);
}

node* APPfor(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfor");

    symbol_table* old_var_table = CURRENT_SYMBOL_TABLE(arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = FOR_VARSYMBOLTABLE(arg_node);

    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if(FOR_STEP(arg_node) != NULL) {
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    }

    if(FOR_CODE(arg_node) != NULL) {
        FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);
    }

    CURRENT_SYMBOL_TABLE(arg_info) = old_var_table;

    DBUG_RETURN(arg_node);
}

node* APPfuncallparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfuncallparameter");
    node* toreturn = arg_node;

    bool prev_infuncall_val = IN_FUNCALL(arg_info);
    node* prev_newnodes = NEWNODES(arg_info);
    PARAMCOUNT(arg_info)++;

    IN_FUNCALL(arg_info) = TRUE;
    NEWNODES(arg_info) = NULL;
    if(FUNCALLPARAMETER_VALUE(arg_node) != NULL) {
        FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);
    }
    IN_FUNCALL(arg_info) = prev_infuncall_val;

    if(NEWNODES(arg_info) != NULL) {
        toreturn = NEWNODES(arg_info);
        append_funcallparam_to_tail(NEWNODES(arg_info), arg_node);
    }

    NEWNODES(arg_info) = prev_newnodes;

    if(FUNCALLPARAMETER_NEXT(arg_node) != NULL) {
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(toreturn);
}

node* APPfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPfuncall");

    size_t prev_paramcount = PARAMCOUNT(arg_info);
    PARAMCOUNT(arg_info) = 0;

    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }

    fun_symbol_table_entry* entry = FUNCALL_FUNPOINTER(arg_node);

    if(PARAMCOUNT(arg_info) != entry->paramamount) {
        CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node),
            "Invalid array dimensions in function call \"%s\"! Expected a total of %zu parameters, but got %zu parameters!",
            FUNCALL_FUNCTIONNAME(arg_node), entry->paramamount, PARAMCOUNT(arg_info));
    }

    PARAMCOUNT(arg_info) = prev_paramcount;

    DBUG_RETURN(arg_node);
}

node* APPvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("APPvarread");

    symbol_table_entry* entry = VARREAD_VARPOINTER(arg_node);

    if(!IN_FUNCALL(arg_info)) {
        DBUG_RETURN(arg_node);
    }

    switch(entry->vartype) {
        case TP_bool_array:
        case TP_float_array:
        case TP_int_array:
            break;
        default:
            DBUG_RETURN(arg_node);
    }

    size_t i = 0;

    while(TRUE) {
        char name_buffer[STR_BUF_SIZE];
        snprintf(name_buffer, STR_BUF_SIZE, "%zu", i++);
        char* dim_var_name = STRcatn(4, "tmp-", name_buffer, "-", entry->name);

        // As we do not track the dimensions in the symbol table, find the dimensions manually
        symbol_table_entry* dim_entry = var_is_in_scope(CURRENT_SYMBOL_TABLE(arg_info), dim_var_name);

        if(dim_entry == NULL) {
            MEMfree(dim_var_name);
            break;
            // No more dimensions
        }

        PARAMCOUNT(arg_info)++;

        node* dim_varread = TBmakeVarread(STRcpy(dim_var_name));
        VARREAD_VARPOINTER(dim_varread) = dim_entry;
        node* new_funcallparam = TBmakeFuncallparameter(TBmakeExpr(dim_varread), NULL);

        if(NEWNODES(arg_info) == NULL) {
            NEWNODES(arg_info) = new_funcallparam;
        } else {
            append_funcallparam_to_tail(NEWNODES(arg_info), new_funcallparam);
        }

        MEMfree(dim_var_name);
    }

    DBUG_RETURN(arg_node);
}

node* APPdo(node* syntaxtree) {
    DBUG_ENTER("APPdo");

    info* arg_info = APPmakeInfo();
    CURRENT_FUNCTION_TABLE(arg_info) = PROGRAM_FUNSYMBOLTABLE(syntaxtree);
    CURRENT_SYMBOL_TABLE(arg_info) = PROGRAM_VARSYMBOLTABLE(syntaxtree);

    TRAVpush(TR_app);

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    TRAVpop();

    arg_info = APPfreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
