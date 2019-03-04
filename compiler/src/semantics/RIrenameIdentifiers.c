#include "RIrenameIdentifiers.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

node *RIglobal(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIglobal");

    if (GLOBAL_DEFINITION(arg_node) != NULL)
        GLOBAL_DEFINITION(arg_node) = TRAVdo(GLOBAL_DEFINITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIglobaldefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIglobaldefinition");

    if (GLOBALDEFINITION_DIMENSIONS(arg_node) != NULL)
        GLOBALDEFINITION_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEFINITION_DIMENSIONS(arg_node), arg_info);

    if (GLOBALDEFINITION_VALUES(arg_node) != NULL)
        GLOBALDEFINITION_VALUES(arg_node) = TRAVdo(GLOBALDEFINITION_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIfunction");

    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    if (FUNCTION_DEFINITION(arg_node) != NULL)
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIfunctiondefinition(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIfunctiondefinition");

    FUNCTIONDEFINITION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTIONDEFINITION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);   
}

node *RIbasicparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIbasicparameter");

    if (BASICPARAMETER_NEXT(arg_node) != NULL)
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIarrayparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIarrayparameter");

    ARRAYPARAMETER_DIMENSIONS(arg_node) = TRAVdo(ARRAYPARAMETER_DIMENSIONS(arg_node), arg_info);

    if (ARRAYPARAMETER_NEXT(arg_node) != NULL)
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIfunctionvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("functionvardeclaration");

    FUNCTIONVARDECLARATION_CURRENT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_CURRENT(arg_node), arg_info);

    if (FUNCTIONVARDECLARATION_NEXT(arg_node) != NULL)
        FUNCTIONVARDECLARATION_NEXT(arg_node) = TRAVdo(FUNCTIONVARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
    
}

node *RIfor(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIfor");

    FOR_COUNTERINIT(arg_node) = TRAVdo(FOR_COUNTERINIT(arg_node), arg_info);

    FOR_CONDITION(arg_node) = TRAVdo(FOR_CONDITION(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);

    FOR_CODE(arg_node) = TRAVdo(FOR_CODE(arg_node), arg_info);

    DBUG_RETURN(arg_node);   
}

node *RIarrayvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIvarassign");

    ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);

    ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIvarassign(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIarrayvarassign");

    VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);

    DBUG_RETURN(arg_node);    
}

node *RIbasicvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("basicvardeclaration");

    if (BASICVARDECLARATION_INITVAL(arg_node) != NULL)
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIarrayvardeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIarrayvardeclaration");

    ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);

    if (ARRAYVARDECLARATION_INITVAL(arg_node) != NULL)
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIfuncall(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIfuncall");

    if (FUNCALL_PARAMS(arg_node) != NULL)
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RIfuncallparameter(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIfuncallparameter");

    FUNCALLPARAMETER_VALUE(arg_node) = TRAVdo(FUNCALLPARAMETER_VALUE(arg_node), arg_info);

    if (FUNCALLPARAMETER_NEXT(arg_node) != NULL)
        FUNCALLPARAMETER_NEXT(arg_node) = TRAVdo(FUNCALLPARAMETER_NEXT(arg_node), arg_info);

        DBUG_RETURN(arg_node);
}

node *RIvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIvarread");

    DBUG_RETURN(arg_node);
}

node *RIarrayvarread(node *arg_node, info *arg_info) {
    DBUG_ENTER("RIarrayvarread");

    ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}


node *RIrenameIdentifiers(node *syntaxtree) {
    DBUG_ENTER("RIrenameIdentifiers");

    TRAVpush(TR_ri);

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}