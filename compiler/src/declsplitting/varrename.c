#include "varrename.h"
#include "symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

#include "util.h"

node* VRNbasicvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNbasicvardeclaration");

    symbol_table_entry* entry = BASICVARDECLARATION_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(BASICVARDECLARATION_VARNAME(arg_node), entry->name)) {
        MEMfree(BASICVARDECLARATION_VARNAME(arg_node));
        BASICVARDECLARATION_VARNAME(arg_node) = STRcpy(entry->name);
    }

    if(BASICVARDECLARATION_INITVAL(arg_node) != NULL) {
        BASICVARDECLARATION_INITVAL(arg_node) = TRAVdo(BASICVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNarrayvardeclaration(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNarrayvardeclaration");

    symbol_table_entry* entry = ARRAYVARDECLARATION_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(ARRAYVARDECLARATION_VARNAME(arg_node), entry->name)) {
        MEMfree(ARRAYVARDECLARATION_VARNAME(arg_node));
        ARRAYVARDECLARATION_VARNAME(arg_node) = STRcpy(entry->name);
    }

    if(ARRAYVARDECLARATION_INITVAL(arg_node) != NULL) {
        ARRAYVARDECLARATION_INITVAL(arg_node) = TRAVdo(ARRAYVARDECLARATION_INITVAL(arg_node), arg_info);
    }

    if(ARRAYVARDECLARATION_DIMENSIONS(arg_node) != NULL) {
        ARRAYVARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(ARRAYVARDECLARATION_DIMENSIONS(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNarrayvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNarrayvarread");

    symbol_table_entry* entry = ARRAYVARREAD_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(ARRAYVARREAD_ARRAYVARNAME(arg_node), entry->name)) {
        MEMfree(ARRAYVARREAD_ARRAYVARNAME(arg_node));
        ARRAYVARREAD_ARRAYVARNAME(arg_node) = STRcpy(entry->name);
    }

    if(ARRAYVARREAD_POSITION(arg_node) != NULL) {
        ARRAYVARREAD_POSITION(arg_node) = TRAVdo(ARRAYVARREAD_POSITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNvarread(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNvarread");
    UNUSED(arg_info);

    symbol_table_entry* entry = VARREAD_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(VARREAD_VARNAME(arg_node), entry->name)) {
        MEMfree(VARREAD_VARNAME(arg_node));
        VARREAD_VARNAME(arg_node) = STRcpy(entry->name);
    }

    DBUG_RETURN(arg_node);
}

node* VRNvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNvarassign");

    symbol_table_entry* entry = VARASSIGN_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(VARASSIGN_VARNAME(arg_node), entry->name)) {
        MEMfree(VARASSIGN_VARNAME(arg_node));
        VARASSIGN_VARNAME(arg_node) = STRcpy(entry->name);
    }

    if(VARASSIGN_VALUE(arg_node) != NULL) {
        VARASSIGN_VALUE(arg_node) = TRAVdo(VARASSIGN_VALUE(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNarrayvarassign(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNarrayvarassign");

    symbol_table_entry* entry = ARRAYVARASSIGN_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(ARRAYVARASSIGN_VARNAME(arg_node), entry->name)) {
        MEMfree(ARRAYVARASSIGN_VARNAME(arg_node));
        ARRAYVARASSIGN_VARNAME(arg_node) = STRcpy(entry->name);
    }

    if(ARRAYVARASSIGN_POSITION(arg_node) != NULL) {
        ARRAYVARASSIGN_POSITION(arg_node) = TRAVdo(ARRAYVARASSIGN_POSITION(arg_node), arg_info);
    }

    if(ARRAYVARASSIGN_VALUES(arg_node) != NULL) {
        ARRAYVARASSIGN_VALUES(arg_node) = TRAVdo(ARRAYVARASSIGN_VALUES(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNbasicparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNbasicparameter");

    symbol_table_entry* entry = BASICPARAMETER_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(BASICPARAMETER_NAME(arg_node), entry->name)) {
        MEMfree(BASICPARAMETER_NAME(arg_node));
        BASICPARAMETER_NAME(arg_node) = STRcpy(entry->name);
    }

    if(BASICPARAMETER_NEXT(arg_node) != NULL) {
        BASICPARAMETER_NEXT(arg_node) = TRAVdo(BASICPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNarrayparameter(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNarrayparameter");

    symbol_table_entry* entry = ARRAYPARAMETER_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(ARRAYPARAMETER_NAME(arg_node), entry->name)) {
        MEMfree(ARRAYPARAMETER_NAME(arg_node));
        ARRAYPARAMETER_NAME(arg_node) = STRcpy(entry->name);
    }

    if(ARRAYPARAMETER_NEXT(arg_node) != NULL) {
        ARRAYPARAMETER_NEXT(arg_node) = TRAVdo(ARRAYPARAMETER_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNarraydimensionvars(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNarraydimensionvars");

    symbol_table_entry* entry = ARRAYDIMENSIONVARS_VARPOINTER(arg_node);
    DBUG_ASSERT((entry != NULL), "Entry pointer not set!");

    if(!STReq(ARRAYDIMENSIONVARS_NAME(arg_node), entry->name)) {
        MEMfree(ARRAYDIMENSIONVARS_NAME(arg_node));
        ARRAYDIMENSIONVARS_NAME(arg_node) = STRcpy(entry->name);
    }

    if(ARRAYDIMENSIONVARS_NEXT(arg_node) != NULL) {
        ARRAYDIMENSIONVARS_NEXT(arg_node) = TRAVdo(ARRAYDIMENSIONVARS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node* VRNfunction(node* arg_node, info* arg_info) {
    DBUG_ENTER("VRNfunction");

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);

        if(FUNCTION_PARAMETERS(arg_node) != NULL) {
            FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
        }
    }

    DBUG_RETURN(arg_node);
}

node* VRNdo(node* syntaxtree) {
    DBUG_ENTER("VRNdo");

    TRAVpush(TR_vrn);

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
