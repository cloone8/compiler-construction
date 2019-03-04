#include "build_symbol_table.h"
#include "symboltable.h"

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
    symbol_table* vars;
};

/*
 * INFO macros
 */
#define GET_ST(__info_struct__) ((__info_struct__)->vars)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *STBmakeInfo(void) {
    DBUG_ENTER("STBmakeInfo");

    info *result;

    result = (info *)MEMmalloc(sizeof(info));

    GET_ST(result) = symbol_table_make();

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *STBfreeInfo(info *info) {
    DBUG_ENTER("STBfreeInfo");

    symbol_table_free(GET_ST(info));
    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *STBglobal(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBglobal");

    if (GLOBAL_DIMENSIONS(arg_node) != NULL)
        GLOBAL_DIMENSIONS(arg_node) = TRAVdo(GLOBAL_DIMENSIONS(arg_node), arg_info);

    if (GLOBAL_DEFINITION(arg_node) != NULL)
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBglobaldefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBglobaldefinition");

    if (GLOBALDEFINITION_VALUES(arg_node) != NULL)
        GLOBALDEFINITION_VALUES(arg_node) = TRAVdo(GLOBALDEFINITION_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunction");

    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    if (FUNCTION_DEFINITION(arg_node) != NULL)
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfunctiondefinition");

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBbasicparameter");

    if (BASICPARAMETER_NEXT(arg_node) != NULL)
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayparameter");

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    if (ARRAYPARAMETER_NEXT(arg_node) != NULL)
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBfunctionvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("functionvardeclaration");

    FUNCTIONVARDECLARATION_CURRENT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    if (FUNCTIONVARDECLARATION_NEXT(arg_node) != NULL)
        FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);

}

node *STBfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfor");

    FOR_COUNTERINIT(arg_node) = TRAVdo(FOR_COUNTERINIT(arg_node), arg_info);

    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

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

    if (BASICVARDECLARATION_INITVAL(arg_node) != NULL)
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *STBarrayvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBarrayvardeclaration");

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

node *STBfuncallparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("STBfuncallparameter");

    FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);

    if (FUNCALLPARAMETER_NEXT(arg_node) != NULL)
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);

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


node *STBdo(node *syntaxtree) {
    DBUG_ENTER("STBdo");

    TRAVpush(TR_stb);
    info* arg_info = STBmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, NULL);

    PROGRAM_SYMBOLTABLE(syntaxtree) = GET_ST(arg_info);

    STBfreeInfo(arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
