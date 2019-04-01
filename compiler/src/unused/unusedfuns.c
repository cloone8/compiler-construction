#include "unusedfuns.h"

#include "dbug.h"
#include "types.h"
#include "count_map.h"
#include "util.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free_node.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

#include "fun_symbol_table.h"

/*
 * INFO structure
 */
struct INFO {
    countMap* funcounts;
    bool deletephase;
    bool deleteinit;
};


/*
 * INFO macros
 */

#define COUNTS(__arg_info_struct__) ((__arg_info_struct__)->funcounts)
#define DELETE(__arg_info_struct__) ((__arg_info_struct__)->deletephase)
#define DELETEINIT(__arg_info_struct__) ((__arg_info_struct__)->deleteinit)

/*
 * INFO functions
 */

/**
 * Creates a new info struct and sets the initial values
 */
static info *SUFmakeInfo(void) {
    DBUG_ENTER("SUFmakeInfo");

    info *result = (info *)MEMmalloc(sizeof(info));
    result->funcounts = CMcreate();
    result->deletephase = FALSE;
    result->deleteinit = FALSE;

    DBUG_RETURN(result);
}

/**
 * Frees the memory of an info struct
 */
static info *SUFfreeInfo(info *info) {
    DBUG_ENTER("SUFfreeInfo");

    CMfree(info->funcounts);
    info = MEMfree(info);

    DBUG_RETURN(info);
}


node *SUFfunction(node *arg_node, info *arg_info) {
    DBUG_ENTER("SUFfunction");

    if(!DELETE(arg_info)) {
        if(STReq(FUNCTION_NAME(arg_node), "__init")) {
            // Init is always guaranteed to have a single statement, as we implicitly add return statements to
            // the end of void-type functions
            node* init_statements = FUNCTIONBODY_STATEMENTS(FUNCTIONDEFINITION_FUNCTIONBODY(FUNCTION_DEFINITION(arg_node)));
            if(STATEMENTS_NEXT(init_statements) != NULL) {
                CMaddOccurence((COUNTS(arg_info)), FUNCTION_NAME(arg_node));
            }
        } else {
            if(FUNCTION_DEFINITION(arg_node) != NULL && FUNCTIONDEFINITION_ISEXPORT(FUNCTION_DEFINITION(arg_node)) == TRUE) {
                // Add an occurence of this item, so exported functions are never deleted
                CMaddOccurence(COUNTS(arg_info), FUNCTION_NAME(arg_node));
            }
        }
    }

    if(FUNCTION_PARAMETERS(arg_node) != NULL) {
        TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    }

    if(FUNCTION_DEFINITION(arg_node) != NULL) {
        TRAVdo(FUNCTION_DEFINITION(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *SUFfuncall(node* arg_node, info* arg_info) {
    DBUG_ENTER("SUFfuncall");

    if(FUNCALL_PARAMS(arg_node) != NULL) {
        FUNCALL_PARAMS(arg_node) = TRAVdo(FUNCALL_PARAMS(arg_node), arg_info);
    }
    if(!DELETE(arg_info)) {
        CMaddOccurence(COUNTS(arg_info), FUNCALL_FUNCTIONNAME(arg_node));
    }
    DBUG_RETURN(arg_node);
}

node *SUFdeclaration(node *arg_node, info *arg_info) {
    DBUG_ENTER("SUFdeclaration");
    node* toreturn = arg_node;

    if(!DELETE(arg_info)) {
        DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);

        if(DECLARATION_NEXT(arg_node) != NULL) {
            DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);
        }
    } else {
        DECLARATION_CURRENT(arg_node) = TRAVdo(DECLARATION_CURRENT(arg_node), arg_info);
        if(NODE_TYPE(DECLARATION_CURRENT(arg_node)) == N_function) {
            countMapItem* item = CMgetItem(COUNTS(arg_info), FUNCTION_NAME(DECLARATION_CURRENT(arg_node)));

            // Delete the function if there are no calls to it, or the function is the init function and
            // its body is empty
            if(item == NULL) {
                char* fun_name = FUNCTION_NAME(DECLARATION_CURRENT(arg_node));
                DBUG_PRINT("SUFfree", ("Deleting function %s", fun_name));

                if(STReq(NODE_FILE(arg_node), global.infile) && !STReq("__init", fun_name)) {
                    CTIwarnFile(NODE_FILE(arg_node), NODE_LINE(arg_node), "Unused non-export function \"%s\"", fun_name);
                }

                fun_symbol_table_entry* entry = FUNCTION_FUNPOINTER(DECLARATION_CURRENT(arg_node));
                fun_symbol_table* table = entry->table;
                fun_symbol_table_remove(table, entry->idx);
                entry = NULL;

                toreturn = DECLARATION_NEXT(arg_node);

                DECLARATION_NEXT(arg_node) = NULL;

                TRAVpush(TR_free);
                FREEdeclaration(arg_node, NULL);
                TRAVpop();

                if(toreturn != NULL) {
                    toreturn = TRAVdo(toreturn, arg_info);
                }

                DBUG_RETURN(toreturn);
            }
        }

        if(DECLARATION_NEXT(arg_node) != NULL) {
            DECLARATION_NEXT(arg_node) = TRAVdo(DECLARATION_NEXT(arg_node), arg_info);
        }
    }

    DBUG_RETURN(toreturn);
}
node *SUFlocalfunction(node* arg_node, info* arg_info) {
    DBUG_ENTER("SUFlocalfunction");
    if(!DELETE(arg_info)) {

    } else {

    }
    DBUG_RETURN(arg_node);
}

node *SUFdo(node * syntaxtree) {
    DBUG_ENTER("SUFdo");

    TRAVpush(TR_suf);
    info *arg_info = SUFmakeInfo();

    syntaxtree = TRAVdo(syntaxtree, arg_info);
    DELETE(arg_info) = TRUE;
    DBUG_PRINT("SUF", ("Starting deletion traversal"));
    syntaxtree = TRAVdo(syntaxtree, arg_info);

    SUFfreeInfo(arg_info);
    TRAVpop();

    if(PROGRAM_FIRST(syntaxtree) == NULL) {
        CTIwarn("The output file wasn't created or modified as no output has been generated.");
        CTIwarnContinued("This is the result of compiling a file which exports no functions.");
        CTIterminateCompilation(syntaxtree);
    }

    DBUG_RETURN(syntaxtree);
}
