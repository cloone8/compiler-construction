#include "build_symbol_table.h"
#include "symbol_table.h"

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

static type type_to_array_type(type current) {
    DBUG_ENTER("type_to_array_type");
    DBUG_ASSERT((current != TP_void && current != TP_unknown), "Invalid type given");

    switch(current) {
        case TP_bool:
            DBUG_RETURN(TP_bool_array);
        case TP_int:
            DBUG_RETURN(TP_int_array);
        case TP_float:
            DBUG_RETURN(TP_float_array);
        default:
            DBUG_RETURN(current);
    }
}

static bool is_in_scope(symbol_table* deepest_table, char* to_find, size_t scope) {
    symbol_table* current_table = deepest_table;

    for(size_t scopes_left = scope; scopes_left > 0; scopes_left--) {
        symbol_table_entry* result = symbol_table_get_name(current_table, to_find);

        if(result != NULL)
            return TRUE;

        current_table = symbol_table_move_up(current_table, 1);
    }

    return FALSE;
}

node *STBglobal(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBglobal");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    if (GLOBAL_DIMENSIONS(arg_node) != NULL) {
        type globaltype = type_to_array_type(GLOBAL_TYPE(arg_node));
        symbol_table_add(current, GLOBAL_NAME(arg_node), globaltype);

        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);
    } else {
        symbol_table_add(current, GLOBAL_NAME(arg_node), GLOBAL_TYPE(arg_node));
    }

    if (GLOBAL_DEFINITION(arg_node) != NULL) {
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *STBfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunction");

    if (FUNCTION_DEFINITION(arg_node) != NULL) {
        PARENT_FUNCTION(arg_info) = arg_node;
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *STBfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunctiondefinition");

    node* function_declaration = PARENT_FUNCTION(arg_info);
    DBUG_ASSERT((function_declaration != NULL), "No parent function found");
    PARENT_FUNCTION(arg_info) = NULL;

    symbol_table* new_scope = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info));
    symbol_table* old_scope = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = new_scope;
    CURRENT_SCOPE(arg_info)++;

    if (FUNCTION_PARAMETERS(function_declaration) != NULL) {
        FUNCTION_PARAMETERS(function_declaration) = TRAVdo(FUNCTION_PARAMETERS(function_declaration), arg_info);
    }

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_scope;
    CURRENT_SCOPE(arg_info)--;

    DBUG_RETURN(arg_node);
}

node *STBbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBbasicparameter");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_add(current, BASICPARAMETER_NAME(arg_node), BASICPARAMETER_TYPE(arg_node));

    if (BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *STBarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayparameter");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    type atype = type_to_array_type(ARRAYPARAMETER_TYPE(arg_node));

    symbol_table_add(current, ARRAYPARAMETER_NAME(arg_node), atype);

    if (ARRAYPARAMETER_NEXT(arg_node) != NULL)
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfor");

    symbol_table* new_scope = symbol_table_make(CURRENT_SYMBOL_TABLE(arg_info));
    symbol_table* old_scope = CURRENT_SYMBOL_TABLE(arg_info);
    CURRENT_SYMBOL_TABLE(arg_info) = new_scope;
    CURRENT_SCOPE(arg_info)++;

    FOR_COUNTER(arg_node) = TRAVdo(FOR_COUNTER(arg_node), arg_info);

    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    CURRENT_SYMBOL_TABLE(arg_info) = old_scope;
    CURRENT_SCOPE(arg_info)--;

    DBUG_RETURN(arg_node);
}

node *STBarrayvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBvarassign");

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvarassign");

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBbasicvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("basicvardeclaration");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);

    symbol_table_add(current, BASICVARDECLARATION_VARNAME(arg_node), BASICVARDECLARATION_TYPE(arg_node));

    if (BASICVARDECLARATION_INITVAL(arg_node) != NULL)
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBarrayvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvardeclaration");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    type atype = type_to_array_type(ARRAYVARDECLARATION_TYPE(arg_node));
    symbol_table_add(current, ARRAYVARDECLARATION_VARNAME(arg_node), atype);

    ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);

    if (ARRAYVARDECLARATION_INITVAL(arg_node) != NULL)
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfuncall");

    if (FUNCALL_PARAMS(arg_node) != NULL)
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBvarread");

    DBUG_RETURN(arg_node);
}

node *STBarrayvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvarread");

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBarraydimensionvars(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarraydimensionvars");

    symbol_table* current = CURRENT_SYMBOL_TABLE(arg_info);
    symbol_table_add(current, ARRAYDIMENSIONVARS_NAME(arg_node), TP_int);

    ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBdo(node *syntaxtree) {
    DBUG_ENTER("STBdo");

    TRAVpush(TR_stb);
    info* arg_info = STBmakeInfo();

    symbol_table* global_var_table = symbol_table_make(NULL);
    PROGRAM_VARSYMBOLTABLE(syntaxtree) = global_var_table;
    CURRENT_SYMBOL_TABLE(arg_info) = global_var_table;

    syntaxtree = TRAVdo(syntaxtree, arg_info);

    STBfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
