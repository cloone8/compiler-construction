#include "build_symbol_table.h"
#include "symbol_table.h"

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
struct INFO {
    symbol_table* current_table;
    node* function_parent_node;
    size_t current_scope;
};

/*
 * INFO macros
 */

#define CURRENT_SYMBOL_TABLE(__arg_info_struct__) ((__arg_info_struct__)->current_table)
#define PARENT_FUNCTION(__arg_info_struct__) ((__arg_info_struct__)->function_parent_node)
#define CURRENT_SCOPE(__arg_info_struct__) ((__arg_info_struct__)->current_scope)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *STBmakeInfo(void) {
    DBUG_ENTER("STBmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    CURRENT_SYMBOL_TABLE(result) = NULL;
    PARENT_FUNCTION(result) = NULL;
    CURRENT_SCOPE(result) = 0;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *STBfreeInfo(info *info) {
    DBUG_ENTER("STBfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/**
 * This function fills the symbol table on a global level. We check if the global
 * is an array or a regular variable.
 * @param arg_node: The current global node to be evaluated.
 * @param arg_info: The INFO struct which contains various information.
 */
node *STBglobal(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBglobal");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    symbol_table_entry* entrypointer;

    if (GLOBAL_DIMENSIONS(arg_node) != NULL) {
        type globaltype = type_to_array_type(GLOBAL_TYPE(arg_node));
        GLOBAL_TYPE(arg_node) = globaltype;
        entrypointer = symbol_table_add(current, GLOBAL_NAME(arg_node), globaltype);

        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
    } else {
        entrypointer = symbol_table_add(current, GLOBAL_NAME(arg_node), GLOBAL_TYPE(arg_node));
    }

    if (GLOBAL_DEFINITION(arg_node) != NULL) {
        GLOBALDEFINITION_VARPOINTER(GLOBAL_DEFINITION(arg_node)) = entrypointer;
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/**
 * In this function we store the parent function node in the INFO struct.
 * 
 * @param arg_node: The current function node.
 * @param arg_info: The INFO struct which contains various information. * 
 **/
node *STBfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunction");

    if (FUNCTION_DEFINITION(arg_node) != NULL) {
        PARENT_FUNCTION(arg_info) = arg_node;
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/**
 * In this function we create the variable symbol table for the current function.
 * 
 * @param arg_node: The current functiondefinition to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 **/
node *STBfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunctiondefinition");

    node* function_declaration = PARENT_FUNCTION(arg_info);
    DBUG_ASSERT((function_declaration != NULL), "No parent function found");
    PARENT_FUNCTION(arg_info) = NULL;

    symbol_table* new_scope = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info), ++CURRENT_SCOPE(arg_info));

    FUNCTIONDEFINITION_VARSYMBOLTABLE(arg_node) = new_scope;

    symbol_table* old_scope = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = new_scope;

    if (FUNCTION_PARAMETERS(function_declaration) != NULL) {
        FUNCTION_PARAMETERS(function_declaration) = TRAVdo(FUNCTION_PARAMETERS(function_declaration), arg_info);
    }

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_scope;
    CURRENT_SCOPE(arg_info)--;

    DBUG_RETURN(arg_node);
}

/**
 * In this function we add basic parameters to their corresponding function table. When more parameters are present
 * we need to traverse these as well.
 * 
 * @param arg_node: The current basic parameter node to be eveluated.
 * @param arg_info: The INFO struct that contains additional information. * 
 **/
node *STBbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBbasicparameter");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_entry* entry = symbol_table_add(current, BASICPARAMETER_NAME(arg_node), BASICPARAMETER_TYPE(arg_node));
    BASICPARAMETER_VARPOINTER(arg_node) = entry;

    if (BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/**
 * In this function we add the symbol table for array parameters to their corresponding symboltable.
 * 
 * @params arg_node: the current array parameter node to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 * 
 */
node *STBarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayparameter");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    type atype = type_to_array_type(ARRAYPARAMETER_TYPE(arg_node));
    ARRAYPARAMETER_TYPE(arg_node) = atype;

    symbol_table_entry* entry = symbol_table_add(current, ARRAYPARAMETER_NAME(arg_node), atype);
    ARRAYPARAMETER_VARPOINTER(arg_node) = entry;

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    /* Traverse further if we have more parameters. */
    if (ARRAYPARAMETER_NEXT(arg_node) != NULL)
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * This function handles the symbol table for the for-loop. Note the special case for the counter.
 * 
 * @params arg_node: The current for-loop node to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 * 
 **/
node *STBfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfor");

    symbol_table* new_scope = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info), ++CURRENT_SCOPE(arg_info));

    FOR_VARSYMBOLTABLE(arg_node) = new_scope;

    symbol_table* old_scope = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = new_scope;

    /* Although the counter is not mandatory in our AST, it should be for the user.
       We lift the counter out of the FOR loop in a later stage. */
    DBUG_ASSERT((FOR_COUNTER(arg_node) != NULL), "FOR counter is NULL!");
    FOR_COUNTER(arg_node) = TRAVdo(FOR_COUNTER(arg_node), arg_info);

    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_scope;
    CURRENT_SCOPE(arg_info)--;

    DBUG_RETURN(arg_node);
}

/**
 * In this function we do not add something to the symbol table. Instead we check if the assignment
 * is valid, based on the scoping rules (we leave types for later).
 * 
 * @params arg_node: The current array variable assignment to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 * 
 **/
node *STBarrayvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBvarassign");

    symbol_table *current = CURRENT_SYMBOL_TABLE(arg_info);
    char* name = ARRAYVARASSIGN_VARNAME(arg_node);

    symbol_table_entry *entry_pointer = var_is_in_scope(current, name);

    if (entry_pointer == NULL) {
        CTIerror("Var %s not in scope", name);
    } else {
        ARRAYVARASSIGN_VARPOINTER(arg_node) = entry_pointer;
    }

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * In this function we do not add something to the symbol table. Instead we check if the assignment
 * is valid, based on the scoping rules (we leave types for later).
 * 
 * @params arg_node: The current variable assignment to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 * 
 **/
node *STBvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvarassign");

    symbol_table *current = CURRENT_SYMBOL_TABLE(arg_info);
    char *name = VARASSIGN_VARNAME(arg_node);

    symbol_table_entry *entry_pointer = var_is_in_scope(current, name);

    if (entry_pointer == NULL) {
        CTIerror("Var %s not in scope", name);
    } else {
        VARASSIGN_VARPOINTER(arg_node) = entry_pointer;
    }

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * In this function we add a basic variable declaration to the symbol table.
 * 
 * @params arg_node the current basic variable declaration to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 **/
node *STBbasicvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("basicvardeclaration");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_entry* entry = symbol_table_add(current, BASICVARDECLARATION_VARNAME(arg_node), BASICVARDECLARATION_TYPE(arg_node));
    BASICVARDECLARATION_VARPOINTER(arg_node) = entry;

    /* If the variable is also initialized, we need to traverse this. */
    if (BASICVARDECLARATION_INITVAL(arg_node) != NULL)
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * In this function we add an array variable declaration to the symbol table.
 * 
 * @params arg_node the current arrat variable declaration to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 **/
node *STBarrayvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvardeclaration");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    type atype = type_to_array_type(ARRAYVARDECLARATION_TYPE(arg_node));
    ARRAYVARDECLARATION_TYPE(arg_node) = atype;

    symbol_table_entry* entry = symbol_table_add(current, ARRAYVARDECLARATION_VARNAME(arg_node), atype);
    ARRAYVARDECLARATION_VARPOINTER(arg_node) = entry;

    /* Traverse dimensions. */
    ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);

    /* If the array is also initialized, we need to traverse this. */
    if (ARRAYVARDECLARATION_INITVAL(arg_node) != NULL)
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * In this function we do not add something to the symboltable. Instead we check if the variable to be read
 * is in scope. If this is the case we set the entrypointer attribute to its corresponding declaration.
 * 
 * @params arg_node: The current variable read to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 **/
node *STBvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBvarread");

    symbol_table *current = CURRENT_SYMBOL_TABLE(arg_info);
    char* name = VARREAD_VARNAME(arg_node);

    symbol_table_entry *entry_pointer = var_is_in_scope(current, name);

    if (entry_pointer == NULL) {
        CTIerror("Var %s not in scope", name);
    } else {
        VARREAD_VARPOINTER(arg_node) = entry_pointer;
    }

    DBUG_RETURN(arg_node);
}

/**
 * In this function we do not add something to the symboltable. Instead we check if the arrayvariable to be read
 * is in scope. If this is the case we set the entrypointer attribute to its corresponding declaration.
 * 
 * @params arg_node: The current arrayvariable read to be evaluated.
 * @param arg_info: The INFO struct that contains additional information.
 **/
node *STBarrayvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvarread");

    symbol_table *current = CURRENT_SYMBOL_TABLE(arg_info);
    char* name = ARRAYVARREAD_ARRAYVARNAME(arg_node);

    symbol_table_entry *entry_pointer = var_is_in_scope(current, name);

    if (entry_pointer == NULL) {
        CTIerror("Var %s not in scope", name);
    }
    else {
        ARRAYVARREAD_VARPOINTER(arg_node) = entry_pointer;
    }

    /* Traverse for the read at a position. */
    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * This traversal is to save the dimentions of an array into the symbol table.
 * 
 * @params arg_node: The current array dimension node to be evaluated. 
 * @param arg_info: The INFO struct that contains additional information. 
 **/
node *STBarraydimensionvars(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarraydimensionvars");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_entry* entry = symbol_table_add(current, ARRAYDIMENSIONVARS_NAME(arg_node), TP_int);
    ARRAYDIMENSIONVARS_VARPOINTER(arg_node) = entry;

    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL)
        ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYDIMENSIONVARS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * The start traversal function.
 * 
 * @params syntaxtree: The current syntaxtree of the program.
 **/
node *STBdo(node *syntaxtree) {
    DBUG_ENTER("STBdo");

    TRAVpush(TR_stb);
    info* arg_info = STBmakeInfo();

    /* Create the global symbol table. */
    symbol_table* global_var_table = symbol_table_make(NULL, 0);
    PROGRAM_VARSYMBOLTABLE(syntaxtree) = global_var_table;
    CURRENT_SYMBOL_TABLE(arg_info) = global_var_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    STBfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
