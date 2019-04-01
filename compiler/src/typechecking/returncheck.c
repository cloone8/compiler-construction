#include "returncheck.h"
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
    bool hasreturn;
    bool isvoidfun;
};


/*
 * INFO macros
 */

#define HASRETURN(__arg_info_struct__) ((__arg_info_struct__)->hasreturn)
#define ISVOID(__arg_info_struct__) ((__arg_info_struct__)->isvoidfun)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *RCmakeInfo(void) {
    DBUG_ENTER("RCmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    HASRETURN(result) = FALSE;
    ISVOID(result) = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *RCfreeInfo(info *info) {
    DBUG_ENTER("RCfreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *RCreturn(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCreturn");

    HASRETURN(arg_info) = TRUE;

    DBUG_RETURN(arg_node);
}

node *RCfunctionbody(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfunctionbody");

    if(FUNCTIONBODY_STATEMENTS(arg_node) != NULL) {
        FUNCTIONBODY_STATEMENTS(arg_node) = TRAVdo(FUNCTIONBODY_STATEMENTS(arg_node), arg_info);
    }

    if(!HASRETURN(arg_info) && ISVOID(arg_info)) {
        node* return_statement = TBmakeStatements(TBmakeReturn(NULL), NULL);
        if(FUNCTIONBODY_STATEMENTS(arg_node) == NULL) {
            FUNCTIONBODY_STATEMENTS(arg_node) = return_statement;
        } else {
            append_statement_to_tail(FUNCTIONBODY_STATEMENTS(arg_node), return_statement);
        }
    }

    if(FUNCTIONBODY_LOCALFUNCTIONS(arg_node) != NULL) {
        FUNCTIONBODY_LOCALFUNCTIONS(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTIONS(arg_node), NULL);
    }

    DBUG_RETURN(arg_node);
}

node *RCfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCfunction");
    UNUSED_VAR(arg_info);

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        info* new_info = RCmakeInfo();
        ISVOID(new_info) = (FUNCTION_TYPE(arg_node) == TP_void);

        FUNCTION_DEFINITION(arg_node) = TRAVdo(FUNCTION_DEFINITION(arg_node), new_info);

        if(!HASRETURN(new_info) && (FUNCTION_TYPE(arg_node) != TP_void)) {
            CTIerrorFile(NODE_FILE(arg_node), NODE_LINE(arg_node),
            "Not all possible branches end in a return statement in function %s", FUNCTION_NAME(arg_node));
        }

        RCfreeInfo(new_info);
    }

    DBUG_RETURN(arg_node);
}

node *RCstatements(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCstatements");

    // Traverse reverse order
    if(STATEMENTS_NEXT(arg_node) != NULL) {
        STATEMENTS_NEXT(arg_node) = TRAVdo(STATEMENTS_NEXT(arg_node), arg_info);
    }

    if(!HASRETURN(arg_info)) {
        STATEMENTS_CURRENT(arg_node) = TRAVdo(STATEMENTS_CURRENT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *RCbranch(node *arg_node, info *arg_info) {
    DBUG_ENTER("RCbranch");

    info* arg_if = RCmakeInfo();
    info* arg_else = RCmakeInfo();

    BRANCH_THENBRANCH(arg_node) = TRAVdo(BRANCH_THENBRANCH(arg_node), arg_if);

    if(BRANCH_ELSEBRANCH(arg_node) != NULL) {
        BRANCH_ELSEBRANCH(arg_node) = TRAVdo(BRANCH_ELSEBRANCH(arg_node), arg_else);
    }

    HASRETURN(arg_info) = (HASRETURN(arg_if) && HASRETURN(arg_else));

    RCfreeInfo(arg_if);
    RCfreeInfo(arg_else);

    DBUG_RETURN(arg_node);
}

node *RCdo(node * syntaxtree) {
    DBUG_ENTER("RCdo");

    TRAVpush(TR_rc);

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
