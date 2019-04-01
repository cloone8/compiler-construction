#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "util.h"

#include "build_function_table.h"

#include "fun_symbol_table.h"

/*
 * INFO structure
 */
struct INFO {
    fun_symbol_table* current_table;
    size_t paramcount;
    type* currentparams;
    size_t current_scope;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define CURRENT_SCOPE(__arg_info_struct__) ((__arg_info_struct__)->current_scope)
#define PARAMS(__arg_info_struct__) ((__arg_info_struct__)->currentparams)
#define PARAMCOUNT(__arg_info_struct__) ((__arg_info_struct__)->paramcount)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 * @return A new and heap-allocated info struct
 */
static info *FTBmakeInfo(void) {
    DBUG_ENTER("FTBmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;
    CURRENT_SCOPE(result) = 0;
    PARAMS(result) = NULL;
    PARAMCOUNT(result) = 0;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct.
 *
 * @param info: The info struct to free.
 * @return NULL
 */
static info *FTBfreeInfo(info *info) {
    DBUG_ENTER("FTBfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/** <!--******************************************************************-->
 *
 * @fn FTBfunction
 *
 * @brief Adds the function and its sons to the symbol table
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *FTBfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBfunction");

    // Go through the parameters. The parameters add their type to the
    // arg_info struct
    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    type* parameters = PARAMS(arg_info);
    size_t paramcount = PARAMCOUNT(arg_info);
    bool isimport = (FUNCTION_DEFINITION(arg_node) == NULL);
    bool isexport = ((FUNCTION_DEFINITION(arg_node) != NULL) && (FUNCTIONDEFINITION_ISEXPORT(FUNCTION_DEFINITION(arg_node))));

    // Assert that the function either has zero parameters, or the parameter
    // type array is not NULL
    DBUG_ASSERT((paramcount == 0 || (paramcount != 0 && parameters != NULL)), "Something went wrong building the function table!");

    PARAMS(arg_info) = NULL;
    PARAMCOUNT(arg_info) = 0;

    fun_symbol_table_entry* entry = fun_symbol_table_add(CURRENT_SYMBOL_TABLE(arg_info), FUNCTION_NAME(arg_node),
                                                         FUNCTION_TYPE(arg_node), paramcount, parameters, isimport, isexport);
    FUNCTION_FUNPOINTER(arg_node) = entry;

    // Go to the function definition, as this function might have local functions
    if(!isimport) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn FTBfunctiondefinition
 *
 * @brief Creates a new symbol table for this function
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *FTBfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBfunctionbody");

    // Create new function symbol table for this function
    fun_symbol_table* new_scope = fun_symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info), ++CURRENT_SCOPE(arg_info));
    FUNCTIONDEFINITION_FUNSYMBOLTABLE(arg_node) = new_scope;

    // Save the old symbol table, so we can restore it when we escape this functions
    // scope
    fun_symbol_table* old_scope = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = new_scope;

    // Traverse through this function body, to find any local functions
    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    // Restore the old scope
    CURRENT_SYMBOL_TABLE(arg_info) = old_scope;
    CURRENT_SCOPE(arg_info)--;

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn FTBbasicparameter
 *
 * @brief Adds this parameters type to the arg_info struct
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *FTBbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBbasicparameter");

    // Increment the amount of parameters by one, and save the index of the
    // current parameter
    size_t current_param = PARAMCOUNT(arg_info)++;

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    } else {
        // This is the last parameter, we know the paramcount and can now
        // allocate the memory for the type-array
        PARAMS(arg_info) = MEMmalloc(PARAMCOUNT(arg_info) * sizeof(type));
    }

    // Fill in the type of this parameter
    PARAMS(arg_info)[current_param] = BASICPARAMETER_TYPE(arg_node);

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn FTBarrayparameter
 *
 * @brief Adds this parameters type to the arg_info struct
 *
 * @param arg_node: node to process
 * @param arg_info: pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *FTBarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("FTBarrayparameter");

    // Increment the amount of parameters by one, and save the index of the
    // current parameter
    size_t current_param = PARAMCOUNT(arg_info)++;

    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    } else {
        // This is the last parameter, we know the paramcount and can now
        // allocate the memory for the type-array
        PARAMS(arg_info) = MEMmalloc(PARAMCOUNT(arg_info) * sizeof(type));
    }

    // Fill in the type of this parameter. As the parser doesn't
    // distinguish between array and basic types, we do that here
    type atype = type_to_array_type(ARRAYPARAMETER_TYPE(arg_node));
    PARAMS(arg_info)[current_param] = atype;

    DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn FTBdo
 *
 * @brief Adds all the functions in the program to their symbol tables
 *
 * @param syntaxree: syntaxtree to process
 *
 * @return processed syntaxtree
 *
 ***************************************************************************/
node *FTBdo(node *syntaxtree) {
    DBUG_ENTER("FTBdo");

    TRAVpush(TR_ftb);
    info* arg_info = FTBmakeInfo();

    // Create the global function table for top level functions
    fun_symbol_table* global_fun_table = fun_symbol_table_make(NULL, 0);
    PROGRAM_FUNSYMBOLTABLE(syntaxtree) = global_fun_table;
    CURRENT_SYMBOL_TABLE(arg_info) = global_fun_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    FTBfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
